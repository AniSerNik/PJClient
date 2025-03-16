// Copyright [2025] Name <email>

#include <common.h>
#include <pins_assignment.h>
#include <lcd.h>
#include <filesystem.h>
#include <lora.h>
//
#include <configmode.h>

static uint64_t timerdebug;  ///< Время, прошедшее с последней команды в конфигурационном режиме
static bool iscfgmode = false;    ///< Статус активности конфигурационном режиме
static bool isauth = false;       ///< Статус авторизации в конфигурационном режиме

/** @name handlers */
/**@{*/
static void cfgmode_handler_help(String params);  
static void cfgmode_handler_exit(String params);
static void cfgmode_handler_print_setmode(String params);

static void cfgmode_handler_server_clearkeys(String params);
//fs
static void cfgmode_handler_fs_size(String params);
static void cfgmode_handler_fsconfig_print(String params);
static void cfgmode_handler_fsconfig_clear(String params);
static void cfgmode_handler_fsconfig_setid(String params);
static void cfgmode_handler_fsconfig_setgateway(String params);
static void cfgmode_handler_fsconfig_setdeepsleep(String params);
//lora
static void cfgmode_handler_lora(String params);
// password
static void cfgmode_handler_fsconfig_setpassword(String params);
/**@}*/

static configmode_commands_t main_commands[] = {
  {"help", cfgmode_handler_help},
  {"exit", cfgmode_handler_exit},
  {"printmode", cfgmode_handler_print_setmode},

  {"serverclearkeys", cfgmode_handler_server_clearkeys},
  //fs
  {"fssize", cfgmode_handler_fs_size},
  {"fscfgprint", cfgmode_handler_fsconfig_print},
  {"fscfgclear", cfgmode_handler_fsconfig_clear},
  {"setid", cfgmode_handler_fsconfig_setid},
  {"setgateway", cfgmode_handler_fsconfig_setgateway},
  {"setsleep", cfgmode_handler_fsconfig_setdeepsleep},
  {"lora", cfgmode_handler_lora},
  // pass
  {"setpass", cfgmode_handler_fsconfig_setpassword}
};

void cfgmode_init() {
  pinMode(CONFIGMODE_PIN, INPUT_PULLDOWN);
  delay(20);
}

void cfgmode_checkbutton() {
  if(digitalRead(CONFIGMODE_PIN) == HIGH)
    cfgmode_enable();
}

void cfgmode_enable() {
  lcd_on();
  iscfgmode = true;
  timerdebug = millis();
  Serial.println("Режим конфигурации");
  Serial.println("Введите пароль");
  lcd_printstatus("CONFIG MODE");
  lcd_print("Need auth", 0, 1);

  while(iscfgmode) {
    if (Serial.available() > 0)
      cfgmode_processcommand(Serial.readStringUntil('\n'));
    if((millis() - timerdebug) > (CONFIGMODE_TIME * 1000)) {
      Serial.println("Режим конфигурации отключен. Причина: Timeout");
      lcd_printstatus("CONFIG MODE off");
      lcd_print("Reason: Timeout", 0, 2);
      iscfgmode = false;
    }
    delay (100); // Для снижения нагрузки на микроконтроллер
    ESP_ERROR_CHECK (esp_task_wdt_reset());
  }
  
  exitProgram ();
}

void cfgmode_processcommand(String command) {
  timerdebug = millis();
  command.trim();

  if(cfgmode_isauth() || command == "exit") {
    Serial.println(command);
    size_t main_commands_size = sizeof(main_commands) / sizeof(main_commands[0]);
    int callcommand = cfgmode_call_command(command, main_commands, main_commands_size);
    if(callcommand == CFG_CALLCOMMAND_NOTFOUND) {
      Serial.println("Неверная команда");
    }
  } else {
    cfgmode_auth(command);
  }
}

int cfgmode_call_command(String str, configmode_commands_t cfg_commands[], size_t size) {
  for(int i = 0; i < size; i++) {
    int32_t index = str.indexOf(cfg_commands[i].cmd);
    if(index == 0) {
      str.remove(index, cfg_commands[i].cmd.length() + 1);
      str.trim();
      cfg_commands[i].handler(str);
      return CFG_CALLCOMMAND_SUCCESS;
    }
  }
  return CFG_CALLCOMMAND_NOTFOUND;
}

bool cfgmode_isauth() {
  return isauth;
}

void cfgmode_auth(String pass) {
  const char *correct_password = fsGetConfigParam<const char*>(FSCONFIGNAME_CFGMODEPASS);
  if (correct_password == NULL || strcmp (correct_password, "") == 0)
    correct_password = CONFIGMODE_DEFPASS;
  Serial.println (pass);
  if(pass.compareTo(correct_password) == 0) {
    isauth = true;
    Serial.println("Авторизация успешна");
    Serial.println("Введите 'help' для получения дополнительной информации");
    lcd_print("Print 'help'", 0, 1);
    return;
  }
  Serial.println("Неверный пароль");
}

/* ***** handlers ***** */
static void cfgmode_handler_help(String params) {
  //Вывести usage каждой команды (третье поле) и все если help пустой. Пример help exit - usage of exit; help - all helps
  Serial.println("\nСписок команд"); //
  Serial.println("exit\t\tВыход из режима отладки");
  Serial.println("setpass X\t\tЗадает пароль для конфигурационного режима");
  Serial.println("setid X\t\tЗадает id устройства");
  Serial.println("setgateway X\tЗадает id шлюза");
  Serial.println("setsleep X\tЗадает время сна устройства. Задавать в секундах");
  Serial.println("printmode X\tЗадает вывод в Serial порт");
  Serial.println("serverclearkeys\tЗапрос очистки ключей на сервере");
  Serial.println("fssize\t\tВывод размера файловой системы");
  Serial.println("fscfgprint\tВывод конфигурационного файла");
  Serial.println("fscfgclear\tОчистка конфигурационного файла");
  Serial.println("lora [param] [value] Задает параметры lora");
  Serial.println("[param] \tsf (spreading factor) | cr (coding rate) | bw (bandwidth)");
  Serial.println();
}

static void cfgmode_handler_exit(String params) {
  Serial.println("Esp sleep...");
  iscfgmode = false;
}

static void cfgmode_handler_print_setmode(String params) {
  uint8_t scanid = 0;
  if(sscanf(params.c_str(), "%" SCNu8, &scanid) > 0 && scanid >= 0 & scanid <= 3) {
    rtcspecmode.encprint = false;
    rtcspecmode.debugprint = false;
    if(scanid == 1 || scanid == 3)
      rtcspecmode.debugprint = true;
    if(scanid == 2 || scanid == 3)
      rtcspecmode.encprint = true;
  }
  else
    Serial.println("printmode ЧИСЛО - Значения от 0 до 3; 0 - Минимальный вывод (По умолчанию); 1 - Основной вывод; 2 - Информация о кодировке json; 3 - Полная информация");
}

static void cfgmode_handler_server_clearkeys(String params) {
  bool olddata = rtcspecmode.debugprint;
  rtcspecmode.debugprint = true;
  loraSendClearData() ? Serial.println("Данные очищены") : Serial.println("Ошибка запроса");
  rtcspecmode.debugprint = olddata;
}
//fs
static void cfgmode_handler_fs_size(String params) {
  fsPrintSize();
}
static void cfgmode_handler_fsconfig_print(String params) {
  fsPrintConfigFile();
}
static void cfgmode_handler_fsconfig_clear(String params) {
  fsClearConfigFile(FSCONFIGFILE) ? Serial.println("Конфиг файл очищен") : Serial.println("Ошибка очистки файла");
}
static void cfgmode_handler_fsconfig_setid(String params) {
  uint8_t scanid = 0;
  if(sscanf(params.c_str(), "%" SCNu8, &scanid) > 0 && scanid > 1 & scanid < 200) {
    if(fsSetConfigParam<uint8_t>(FSCONFIGNAME_ID, scanid)) {
      Serial.print("Новый id устройства  - ");
      Serial.println(fsGetConfigParam<uint8_t>(FSCONFIGNAME_ID));
    }
    else
      Serial.println("Ошибка записи параметра");
  }
  else
    Serial.println("setid ЧИСЛО - принимает значения от 1 до 199");
}
static void cfgmode_handler_fsconfig_setgateway(String params) {
  uint8_t scanid = 0;
  if(sscanf(params.c_str(), "%" SCNu8, &scanid) > 0 && scanid > 199 & scanid < 256) {
    if(fsSetConfigParam<uint8_t>(FSCONFIGNAME_GATEWAY, scanid)) {
      Serial.print("Новый id шлюза - ");
      Serial.println(fsGetConfigParam<uint8_t>(FSCONFIGNAME_GATEWAY));
    }
    else
      Serial.println("Ошибка записи параметра");
  }
  else
    Serial.println("setgateway ЧИСЛО - принимает значения от 200 до 255");
}
static void cfgmode_handler_fsconfig_setdeepsleep(String params) {
  uint16_t scanid = 0;
  if(sscanf(params.c_str(), "%" SCNu16, &scanid) > 0 && scanid > 0) {
    if(fsSetConfigParam<uint16_t>(FSCONFIGNAME_DEEPSLEEP, scanid)) {
      Serial.print("Новое время сна  - ");
      Serial.println(fsGetConfigParam<uint16_t>(FSCONFIGNAME_DEEPSLEEP));
    }
    else
      Serial.println("Ошибка записи параметра");
  }
  else
    Serial.println("setdeepsleep ЧИСЛО");
}
//lora
static void cfgmode_handler_lora(String params) {
  if(params.indexOf("bw") == 0) {
    uint32_t scanid = 0;
    if(sscanf(params.c_str(), "bw %u", &scanid) > 0) {
      if(fsSetConfigParam<uint32_t>(FSCONFIGNAME_LORABW, scanid)) {
        Serial.print("Новое значение bandwidth  - ");
        Serial.println(fsGetConfigParam<uint32_t>(FSCONFIGNAME_LORABW));
      }
      else
        Serial.println("Ошибка записи параметра");
    }
    else
      Serial.println("Signal Bandwidth - in HZ");
  } else if(params.indexOf("sf") == 0) {
    uint8_t scanid = 0;
    if(sscanf(params.c_str(), "sf %" SCNu8, &scanid) > 0 && scanid >= 6 && scanid <= 12) {
      if(fsSetConfigParam<uint8_t>(FSCONFIGNAME_LORASF, scanid)) {
        Serial.print("Новое значение Spreading Factor  - ");
        Serial.println(fsGetConfigParam<uint8_t>(FSCONFIGNAME_LORASF));
      }
      else
        Serial.println("Ошибка записи параметра");
    }
    else
      Serial.println("Spreading Factor. Min - 6. Max - 12");
  } else if(params.indexOf("cr") == 0) {
    uint8_t scanid = 0;
    if(sscanf(params.c_str(), "cr %" SCNu8, &scanid) > 0 && scanid >= 5 && scanid <= 8) {
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
  else
    Serial.println("lora [param] [value] Задает параметры lora\n[param]\tsf (spreading factor) | cr (coding rate) | bw (bandwidth)");
}
// password 

static void cfgmode_handler_fsconfig_setpassword(String params) {
  String scanpass = "";
  // cppcheck-suppress invalidscanf
  if(sscanf(params.c_str(), "%s", &scanpass) > 0) {
    if(fsSetConfigParam<const char*>(FSCONFIGNAME_CFGMODEPASS, scanpass.c_str())) {
      Serial.print("Новый пароль - ");
      Serial.println(fsGetConfigParam<const char*>(FSCONFIGNAME_CFGMODEPASS));
    }
    else
      Serial.println("Ошибка записи параметра");
  }
  else
    Serial.println("setpass ПАРОЛЬ (Без пробелов)");
}