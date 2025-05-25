// Copyright [2025] Name <email>

#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <common.h>
#include <pins_assignment.h>
#include <JDE_client.h>
#include <lcd.h>
#include <filesystem.h>
//
#include <lora.h>
#include <HardwareSerial.h>

//lora
static loraConfigAddress loraAddressCur;        ///< Конфигурация адресов LoRa
static bool lora_init = false;                  ///< Статус инициализации драйвера LoRa

uint8_t send_buf[RH_RF95_MAX_MESSAGE_LEN];      ///< Буфер для исходящих пакетов LoRa
uint8_t recv_buf[RH_RF95_MAX_MESSAGE_LEN];      ///< Буфер для входящих пакетов LoRa

static RH_RF95 loraDriver(SPI_CS, LORA_DIO0);
static RHReliableDatagram loraManager(loraDriver);

void loraSetGatewayAddress(uint8_t addr) {
  loraAddressCur.gatewayAddress = addr;
}

void loraSetClientAddress(uint8_t addr) {
  loraAddressCur.clientAddress = addr;
  loraManager.setThisAddress(addr);
}

bool loraInit() {
  if(loraIsInit())
    return true;

  if (loraManager.init()) {
    loraDriver.setTxPower(20, false);
    loraDriver.setFrequency(869.2);

    uint32_t loraBW = fsGetConfigParam<uint32_t>(FSCONFIGNAME_LORABW);
    if (loraBW == 0)  //if not in fs
      loraBW = LORA_BW_DEFAULT;
    uint8_t loraCR = fsGetConfigParam<uint8_t>(FSCONFIGNAME_LORACR);
    if (loraCR == 0)  //if not in fs
      loraCR = LORA_CR_DEFAULT;
    uint8_t loraSF = fsGetConfigParam<uint8_t>(FSCONFIGNAME_LORASF);
    if (loraSF == 0)  //if not in fs
      loraSF = LORA_SF_DEFAULT;
    SerialP1.println("lora param. SF: " + String(loraSF) + "; BW: " + String(loraBW) + "; CR4: " + String(loraCR));
    loraDriver.setSignalBandwidth(loraBW);
    loraDriver.setCodingRate4(loraCR);
    loraDriver.setSpreadingFactor(loraSF);
    loraManager.setHeaderId(rtcspecmode.lastsendloraid);
    //add parameters
    lora_init = true;
  } else {
    Serial.println("lora init failed");
    lcd_printstatus("Error Lora");
  }
  return lora_init;
}

bool loraIsInit() {
  return lora_init;
}

bool loraSendKeys() {
  if (!loraInit()) 
    return false;

  lcd_printstatus("Send keys");
  SerialP1.println("Регистрация ключей");

  memset(send_buf, 0, sizeof(send_buf));
  send_buf[COMMAND] = REGISTRATION;
  send_buf[BYTE_COUNT] = START_PAYLOAD;
  encodeJsonKeys();
  _loraPrintBuf(send_buf);
  SerialP1.println("Передача ключей json на сервер");
  return _loraSendAndGet();
}

bool loraSendData() {
  if (!loraInit()) 
    return false;

  lcd_printstatus("Sending data");
  SerialP1.println("Кодирование JSON");

  memset(send_buf, 0, sizeof(send_buf));
  send_buf[COMMAND] = DATA;
  send_buf[BYTE_COUNT] = START_PAYLOAD;
  encodeJsonForSending();
  _loraPrintBuf(send_buf);
  SerialP1.println("Передача данных json на сервер");
  return _loraSendAndGet();
}

bool loraSendClearData() {
  if (!loraInit()) 
    return false;

  SerialP1.println("Запрос очистки данных о устройстве на сервере");

  memset(send_buf, 0, sizeof(send_buf));
  send_buf[COMMAND] = CLEAR_INFO;
  send_buf[BYTE_COUNT] = START_PAYLOAD;
  return _loraSendAndGet();
  /*
  if(_loraSendAndGet())
    return (recv_buf[recv_buf[BYTE_COUNT] - 1] == REPLY_TRUE);
  return false;
   */
}

bool loraSendFull() {
  if (!loraInit()) 
    return false;
  lcd_printstatus("Start sending");
  parseJsonKeys();

  if(rtcspecmode.clearkeysonboot) {
    if(!loraSendClearData())
      return false;
    rtcspecmode.clearkeysonboot = false;
  }

  if(loraSendData()) {
    if(recv_buf[recv_buf[BYTE_COUNT] - 1] == REPLY_TRUE)
      return true;
    return loraSendKeys();
  }
  return false;
}

void loraSleep() {
  if (!loraIsInit())
    return; 
    
  loraDriver.sleep();
}

bool _loraSendAndGet() {
  uint64_t startTimer = millis();
  if(loraAddressCur.gatewayAddress == 0) {
    Serial.println("Неверный адрес шлюза (=0)");
    return false;
  }
  if (loraManager.sendtoWait(send_buf, send_buf[BYTE_COUNT], loraAddressCur.gatewayAddress)) {
    uint8_t len = sizeof(recv_buf), from; 
    if (loraManager.recvfromAckTimeout(recv_buf, &len, 2000, &from)) {
      if(from == loraAddressCur.gatewayAddress) {
        SerialP1.print("Получен ответ от сервера 0x" + String(loraAddressCur.gatewayAddress, HEX) + ": ");
        _loraPrintBuf(recv_buf);
        rtcspecmode.lastsendloraid += 1;
        Serial.println("Время передачи: " + String(millis() - startTimer) + "ms");
        return true;
      }
    }
    else
      SerialP1.println("Нет ответа");
  }
  else
    SerialP1.println("Ошибка при передаче");
  rtcspecmode.lastsendloraid += 1;
  SerialP1.println("Время передачи: " + String(millis() - startTimer) + "ms");
  return false;
}

void _loraPrintBuf(uint8_t *buf) {
  for (int i = 0; i < buf[BYTE_COUNT]; i++) {
    SerialP1.print(buf[i]);
    SerialP1.print(" ");
  }
  SerialP1.println();
}