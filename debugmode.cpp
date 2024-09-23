#include "common.h"
#include "pins_assignment.h"
#include "lcd.h"
#include "fs.h"
#include "lora.h"
//
#include "debugmode.h"

static long long int timerdebug;
static bool isdebugmode = false;

void debugmode_init() {
  pinMode(DEBUGMODE_PIN, INPUT_PULLUP);
  if(digitalRead(DEBUGMODE_PIN) == LOW)
    debugmode_enable();
}

void debugmode_enable() {
  lcd_on();
  isdebugmode = true;
  timerdebug = millis();
  Serial.println("Debug mode активирован");
  Serial.println("Print 'help' for additional info");

  lcd_printstatus("DEBUG MODE");
  lcd_print("Print 'help'", 0, 1);

  while(isdebugmode) {
    if (Serial.available() > 0)
      debugmode_processcommand(Serial.readStringUntil('\n'));
    if((millis() - timerdebug) > (DEBUGMODE_TIME * 1000)) {
      Serial.println("Debug mode отключен. Причина: Timeout");
      lcd_printstatus("DEBUG MODE off");
      lcd_print("Reason: Timeout", 0, 2);
      isdebugmode = false;
    }
  }
  exitProgram();
}

void debugmode_processcommand(String command) {
  timerdebug = millis();
  command.trim();
  Serial.println(command);
  
  if(command == "help") {
    Serial.println("\nСписок команд"); //
    Serial.println("exit\t\tВыход из режима отладки");
    Serial.println("setid X\t\tЗадает id устройства");
    Serial.println("setgateway X\tЗадает id шлюза");
    Serial.println("setdeepsleep X\tЗадает время сна устройства. Задавать в секундах");
    Serial.println("printmode X\tЗадает вывод в Serial порт");
    Serial.println("clearkeysserver\tЗапрос очистки ключей на сервере");
    Serial.println("fssize\t\tВывод размера файловой системы");
    Serial.println("fsconfigprint\tВывод конфигурационного файла");
    Serial.println("fsconfigclear\tОчистка конфигурационного файла");
    Serial.println("lora [param] [value] Задает параметры lora");
    Serial.println("[param] \tsf (spreading factor) | cr (coding rate) | bw (bandwidth)");
    Serial.println();
  }
  else if(command == "exit") {
    Serial.println("Esp sleep...");
    isdebugmode = false;
  }
  else if(command == "fssize") {
    fsPrintSize();
  }
  else if(command == "fsconfigprint") {
    fsPrintConfigFile();
  }
  else if(command == "fsconfigclear") {
    if(fsClearConfigFile())
      Serial.println("Конфиг файл очищен");
    else
      Serial.println("Ошибка очистки файла");
  }
  else if(command == "clearkeysserver") {
    bool olddata = rtcspecmode.debugprint;
    rtcspecmode.debugprint = true;
    if(loraSendClearData())
      Serial.println("Данные очищены");
    else
      Serial.println("Ошибка запроса");
    rtcspecmode.debugprint = olddata;
  }
  else if(command.indexOf("setid") == 0) {
    uint8_t scanid = 0;
    if(sscanf(command.c_str(), "setid %d", &scanid) > 0 && scanid > 1 & scanid < 200) {
      if(fsSetConfigParam<uint8_t>(FSCONFIGNAME_ID, scanid)) {
        Serial.print("Новый id устройства  - ");
        Serial.println(fsGetConfigParam<uint8_t>(FSCONFIGNAME_ID));
      }
      else
        Serial.println("Ошибка записи параметра");
    }
    else {
      Serial.println("setid ЧИСЛО - принимает значения от 1 до 199");
    }
  }
  else if(command.indexOf("setgateway") == 0) {
    uint8_t scanid = 0;
    if(sscanf(command.c_str(), "setgateway %d", &scanid) > 0 && scanid > 199 & scanid < 256) {
      if(fsSetConfigParam<uint8_t>(FSCONFIGNAME_GATEWAY, scanid)) {
        Serial.print("Новый id шлюза - ");
        Serial.println(fsGetConfigParam<uint8_t>(FSCONFIGNAME_GATEWAY));
      }
      else
        Serial.println("Ошибка записи параметра");
    }
    else {
      Serial.println("setgateway ЧИСЛО - принимает значения от 200 до 255");
    }
  }
  else if(command.indexOf("setdeepsleep") == 0) {
    uint16_t scanid = 0;
    if(sscanf(command.c_str(), "setdeepsleep %d", &scanid) > 0 && scanid > 0) {
      if(fsSetConfigParam<uint16_t>(FSCONFIGNAME_DEEPSLEEP, scanid)) {
        Serial.print("Новое время сна  - ");
        Serial.println(fsGetConfigParam<uint16_t>(FSCONFIGNAME_DEEPSLEEP));
      }
      else
        Serial.println("Ошибка записи параметра");
    }
    else {
      Serial.println("setdeepsleep ЧИСЛО");
    }
  }
  else if(command.indexOf("printmode") == 0) {
    uint8_t scanid = 0;
    if(sscanf(command.c_str(), "printmode %d", &scanid) > 0 && scanid >= 0 & scanid <= 3) {
      rtcspecmode.encprint = false;
      rtcspecmode.debugprint = false;
      if(scanid == 1 || scanid == 3)
        rtcspecmode.debugprint = true;
      if(scanid == 2 || scanid == 3)
        rtcspecmode.encprint = true;
    }
    else {
      Serial.println("printmode ЧИСЛО - Значения от 0 до 3; 0 - Минимальный вывод (По умолчанию); 1 - Основной вывод; 2 - Информация о кодировке json; 3 - Полная информация");
    }
  }
  else if(command.indexOf("lora") == 0) {
    command.remove(0, 5);
    if(command.indexOf("bw") == 0) {
      uint32_t scanid = 0;
      if(sscanf(command.c_str(), "bw %d", &scanid) > 0 && scanid > 0) {
        if(fsSetConfigParam<uint32_t>(FSCONFIGNAME_LORABW, scanid)) {
          Serial.print("Новое значение bandwidth  - ");
          Serial.println(fsGetConfigParam<uint32_t>(FSCONFIGNAME_LORABW));
        }
        else
          Serial.println("Ошибка записи параметра");
      }
      else
        Serial.println("Signal Bandwidth - in HZ");
    }
    else if(command.indexOf("sf") == 0) {
      uint8_t scanid = 0;
      if(sscanf(command.c_str(), "sf %d", &scanid) > 0 && scanid >= 6 && scanid <= 12) {
        if(fsSetConfigParam<uint8_t>(FSCONFIGNAME_LORASF, scanid)) {
          Serial.print("Новое значение Spreading Factor  - ");
          Serial.println(fsGetConfigParam<uint8_t>(FSCONFIGNAME_LORASF));
        }
        else
          Serial.println("Ошибка записи параметра");
      }
      else
        Serial.println("Spreading Factor. Min - 6. Max - 12");
    }
    else if(command.indexOf("cr") == 0) {
      uint8_t scanid = 0;
      if(sscanf(command.c_str(), "cr %d", &scanid) > 0 && scanid >= 5 && scanid <= 8) {
        if(fsSetConfigParam<uint8_t>(FSCONFIGNAME_LORACR, scanid)) {
          Serial.print("Новое значение Coding Rate 4  - ");
          Serial.println(fsGetConfigParam<uint8_t>(FSCONFIGNAME_LORACR));
        }
        else
          Serial.println("Ошибка записи параметра");
      }
      else
        Serial.println("CodingRate4. Min - 5. Max - 8");
    }
    else {
      Serial.println("lora [param] [value] Задает параметры lora\n[param]\tsf (spreading factor) | cr (coding rate) | bw (bandwidth)");
    }
  }
  else
    Serial.println("Неверная команда");
}