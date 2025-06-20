// Copyright [2025] Name <email>

#include <common.h>
#include <pins_assignment.h>
#include <filesystem.h>
#include <deepsleep.h>
#include <lcd.h>
#include <configmode.h>
#include <sensors.h>
#include <lora.h>
#include <JDE_client.h>
#include <HardwareSerial.h>

//Json generate param
#define PARAM_SerialDevice "1"
#define PARAM_Akey "NeKKxx1"
#define PARAM_VersionDevice "test01"

RTC_DATA_ATTR struct rtcsm rtcspecmode;

esp_task_wdt_config_t twdt_config = {
  .timeout_ms = SEC_TO_MS(WDT_TIMEOUT),           ///< Таймаут для WatchDog, после которого он перезапускает плату
  .idle_core_mask = (1 << configNUM_CORES) - 1,   ///< Битовая маска на которых WDT работает. По умолчанию - на всех.
  .trigger_panic = true,                          ///< Флаг для перезагрузки ESP32 при срабатывании WDT
};

void setup() {
  // Настраиваем Watchdog
  ESP_ERROR_CHECK (esp_task_wdt_reconfigure (&twdt_config));
  ESP_ERROR_CHECK (esp_task_wdt_add(NULL));

  Serial.begin (115200);

  delay (500); //for serial
  Serial.println ("\n---");

  //Инициализируем I2C и SPI
  SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  if (!Wire.begin (I2C_SDA, I2C_SCL, I2C_FREQ))
    Serial.println ("I2C: Не инициализировано");

  //Инициализируем дисплей 
  lcd_init();

  //Инициализируем конфигурационный режим
  cfgmode_init();

  //Инициализируем файловую систему
  if (!fsInit ())
    exitProgram ();

  delay (40); 

  //Проверяем Deepsleep
  uint64_t bitMask = 0;
  wakeup_process (&bitMask);
  Serial.println (bitMask);
  if (checkWakeupGPIO(bitMask, CONFIGMODE_PIN))
    cfgmode_enable();
  if (checkWakeupGPIO(bitMask, LCDPIN_BUTTON))
    lcd_on();

  // Проверяем кнопки
  cfgmode_checkbutton();
  lcd_checkbutton();

  lcd_printstatus("Start...");

  //Проверяем и устанавливаем ID устройства и шлюза
  uint8_t gateway_address = fsGetConfigParam<uint8_t>(FSCONFIGNAME_GATEWAY);
  uint8_t nowId = fsGetConfigParam<uint8_t>(FSCONFIGNAME_ID);
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
    exitProgram ();
  }
  Serial.println("ID устройства - " + String(nowId));
  Serial.println("ID шлюза - " + String(gateway_address));
  loraSetClientAddress(nowId);
  loraSetGatewayAddress(gateway_address);
  lcd_print("ID " + String(nowId), 14, 0);

  //Обработка данных с сенсоров
  lcd_printstatus("Read sensors");
  BME280SensorData bme280data, bme280data_2;
  INA219SensorData ina219data;
  float tsens_value;

  bool loopStage = false;
  do {
    if(loopStage)
      delay(2000);
    lcdslider_clear();
    if(getBME280Data(BME280_ADDRESSI2C, &bme280data)) {
      lcdslider_addparam("T:" + String(bme280data.temperature) + "C");
      lcdslider_addparam("P:" + String(bme280data.pressure) + "mm");
      lcdslider_addparam("H:" + String(bme280data.humidity) + "%");
    } else {
      lcdslider_adderror("Error BME280 1");
      Serial.println("Ошибка инициализации BME280 1");
    }
    if(getBME280Data(BME280_ADDRESSI2C_2, &bme280data_2)) {
      lcdslider_addparam("t:" + String(bme280data_2.temperature) + "C");
      lcdslider_addparam("p:" + String(bme280data_2.pressure) + "mm");
      lcdslider_addparam("h:" + String(bme280data_2.humidity) + "%");
    } else {
      lcdslider_adderror("Error BME280 2");
      Serial.println("Ошибка инициализации BME280 2");
    }
    if(getINA219Data(&ina219data)) {
      lcdslider_addparam("V:" + String(ina219data.voltage) + "v");
    } else {
      lcdslider_adderror("Error INA219");
      Serial.println("Ошибка инициализации INA219");
    }
    // Read temperature sensor
    if(getInternalTemperatureData(&tsens_value)) {
      lcdslider_addparam("_t:" + String(tsens_value) + "C");
    } else {
      lcdslider_adderror("Error Int Temp");
      Serial.println("Ошибка внутреннего датчика");
    }

    lcdslider_update();
    loopStage = true;

    ESP_ERROR_CHECK (esp_task_wdt_reset());
  } while(lcd_isButtonPress() && isHaveTimeDS(4000 + LCDTIMECOUNTERSLEEP * TIMEFACTOR_SMALL));

  if(getRemainTimeDeepSleep() > 0)
    exitProgram ();

  // Устанавливаем логирование для библиотеки JDE_client
  JDESetLogging(rtcspecmode.debugprint, rtcspecmode.encprint);

  //Формируем JSON строку для отправки на сервер
  String json;
  json = "{\"system\":{ ";
  json += "\"Akey\":\"" + String(PARAM_Akey) + "\",";
  json += "\"Serial\":\"" + String(PARAM_SerialDevice) + "\", ";
  json += "\"Version\":\"" + String(PARAM_VersionDevice) + "\", ";
  json += "\"intTemp\":\"" + String(tsens_value, 0) + "\"},";  
  json += "\"BME280\":{ ";
  json += "\"temperature\":\"" + String(bme280data.temperature, 6) + "\",";
  json += "\"humidity\":\"" + String(bme280data.humidity, 6) + "\",";
  json += "\"pressure\":\"" + String(bme280data.pressure, 6) + "\"},";  
  json += "\"BME280_2\":{ ";
  json += "\"temperature\":\"" + String(bme280data_2.temperature, 6) + "\",";
  json += "\"humidity\":\"" + String(bme280data_2.humidity, 6) + "\",";
  json += "\"pressure\":\"" + String(bme280data_2.pressure, 6) + "\"},";  
  json += "\"INA219\":{ ";
  json += "\"voltage\":\"" + String(ina219data.voltage) + "\"}";
  json += "}";

  Serial.println (json);

  if(!jsonStringProcess(json)) {
    exitProgram ();
  }

  //Отправляем данные по LoRa
  if (loraSendFull()) {
    lcd_printstatus("Send success");
    Serial.println("Передача данных успешна!");
  } else {
    lcd_printstatus("Send fail");
    Serial.println("Передача данных не удалась");
  }
  //Завершаем работу
  exitProgram ();
}

void loop() {}

void exitProgram() {
  loraSleep();
  sleepAllSensors();
  lcd_goSleep();
  Serial.println("Time work: " + String(millis() + DEEPSLEEP_STARTDELAY));
  startDeepSleep();
}