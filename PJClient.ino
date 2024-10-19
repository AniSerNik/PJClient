#include "common.h"
#include "pins_assignment.h"
#include "fs.h"
#include "deepsleep.h"
#include "lcd.h"
#include "debugmode.h"
#include "sensors.h"
#include "lora.h"
#include "json.h"

//Json generate param
#define PARAM_SerialDevice "1"
#define PARAM_Akey "NeKKxx1"
#define PARAM_VersionDevice "test01"

void setup() {
  Serial.begin(9600);
  //delay(300); //for serial
  Serial.println("\n---");
  
  //Инициализируем файловую систему
  //if (!fsInit())
  //  exitProgram();

  //Инициализируем I2C и SPI
  SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(1000000);
  delay(40); 

  //Инициализируем дисплей и проверяем кнопку
  //lcd_init();

  //Проверяем кнопку для Дебаг режима
  //debugmode_init();

  //Проверяем Deepsleep
  uint64_t bitMask;
  wakeup_process(bitMask);
  if (checkWakeupGPIO(bitMask, DEBUGMODE_PIN))
    debugmode_enable();
  if (checkWakeupGPIO(bitMask, LCDPIN_BUTTON))
    lcd_on();

  lcd_printstatus("Start...");

  //Проверяем и устанавливаем ID устройства и шлюза
  uint8_t gateway_address = 200; //fsGetConfigParam<uint8_t>(FSCONFIGNAME_GATEWAY);
  uint8_t nowId = 4; //fsGetConfigParam<uint8_t>(FSCONFIGNAME_ID);
  if(nowId == 0 || gateway_address == 0) {
    lcd_on();
    if(gateway_address == 0) {
      lcd_printstatus("NEED SET GATE!");
      Serial.println("Необходимо задать адрес шлюза");
    }
    if(nowId == 0) {
      lcd_printstatus("NEED SET ID!");
      Serial.println("Необходимо задать ID устройства");
    }
    delay(3000);
    exitProgram();
  }
  Serial.println("ID устройства - " + String(nowId));
  Serial.println("ID шлюза - " + String(gateway_address));
  loraSetClientAddress(nowId);
  loraSetGatewayAddress(gateway_address);
  //lcd_print("ID " + String(nowId), 14, 0);
  
  //Обработка данных с сенсоров
  //lcd_printstatus("Read sensors");
  BME280SensorData bme280data;
  INA219SensorData ina219data;
  bool loopStage = false;
 //do {
//    if(loopStage)
//      delay(2000);
//    lcdslider_clear();
    if (getBME280Data(&bme280data)) {
//      lcdslider_addparam("T:" + String(bme280data.temperature) + "C");
//      lcdslider_addparam("P:" + String(bme280data.pressure) + "mm");
//      lcdslider_addparam("H:" + String(bme280data.humidity) + "%");
    }
    else {
//      lcdslider_adderror("Error BME280");
      Serial.println("Ошибка инициализации BME280");
    }
    if (getINA219Data(&ina219data)) {}
//      lcdslider_addparam("V:" + String(ina219data.voltage) + "v");
    else {
//      lcdslider_adderror("Error INA219");
      Serial.println("Ошибка инициализации INA219");
    }
//    lcdslider_update();
//    loopStage = true;
//  } while(lcd_isButtonPress() && isHaveTimeDS(4000 + LCDTIMECOUNTERSLEEP * TIMEFACTOR_SMALL));

  if (getRemainTimeDS_left() > 0)
    exitProgram();

  //Формируем JSON строку для отправки на сервер
  String json;
  json = "{\"system\":{ ";
  json += "\"Akey\":\"" + String(PARAM_Akey) + "\",";
  json += "\"Serial\":\"" + String(PARAM_SerialDevice) + "\", ";
  json += "\"Version\":\"" + String(PARAM_VersionDevice) + "\"},";
  json += "\"BME280\":{ ";
  json += "\"temperature\":\"" + String(bme280data.temperature, 6) + "\",";
  json += "\"humidity\":\"" + String(bme280data.humidity, 6) + "\",";
  json += "\"pressure\":\"" + String(bme280data.pressure, 6) + "\"},";  
  json += "\"INA219\":{ ";
  json += "\"voltage\":\"" + String(ina219data.voltage) + "\",";
  json += "\"current\":\"" + String(ina219data.current) + "\",";
  json += "\"power\":\"" + String(ina219data.power) + "\"}";
  json += "}";

  if(!jsonStringProcess(json))
    exitProgram();

  //Отправляем данные по LoRa
  if (loraSendFull()) {
    lcd_printstatus("Send success");
    Serial.println("Передача данных успешна!");
  }
  else {
    lcd_printstatus("Send fail");
    Serial.println("Передача данных не удалась");
  }
  //Завершаем работу
  exitProgram();
}

void loop() {}

void exitProgram() {
  loraSleep();
  sleepAllSensors();
  //lcd_goSleep();
  Serial.println("Time work: " + String(millis() + DEEPSLEEP_STARTDELAY));
  startDeepSleep();
}