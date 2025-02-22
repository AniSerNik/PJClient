#include "common.h"
#include "pins_assignment.h"
#include "filesystem.h"
#include "deepsleep.h"
#include "lcd.h"
#include "configmode.h"
#include "sensors.h"
#include "lora.h"
#include "json.h"
#include <HardwareSerial.h>

//Json generate param
#define PARAM_SerialDevice "1"
#define PARAM_Akey "NeKKxx1"
#define PARAM_VersionDevice "test01"

void setup() {
  Serial.begin (115200);

  delay (500); //for serial
  Serial.println ("\n---");

  enableCore0WDT();

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
    ESP_SLEEP

  delay (40); 

  //Проверяем Deepsleep
  uint64_t bitMask = 0;
  wakeup_process(bitMask);
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
    ESP_SLEEP
  }
  Serial.println("ID устройства - " + String(nowId));
  Serial.println("ID шлюза - " + String(gateway_address));
  loraSetClientAddress(nowId);
  loraSetGatewayAddress(gateway_address);
  lcd_print("ID " + String(nowId), 14, 0);

  //Обработка данных с сенсоров
  lcd_printstatus("Read sensors");
  BME280SensorData bme280data;
  INA219SensorData ina219data;
  float tsens_value;

  bool loopStage = false;
  // WDT не нравиться цикл
  disableCore0WDT();
  do {
    if(loopStage)
      delay(2000);
    lcdslider_clear();
    if(getBME280Data(&bme280data)) {
      lcdslider_addparam("T:" + String(bme280data.temperature) + "C");
      lcdslider_addparam("P:" + String(bme280data.pressure) + "mm");
      lcdslider_addparam("H:" + String(bme280data.humidity) + "%");
    }
    else {
      lcdslider_adderror("Error BME280");
      Serial.println("Ошибка инициализации BME280");
    }
    if(getINA219Data(&ina219data))
      lcdslider_addparam("V:" + String(ina219data.voltage) + "v");
    else {
      lcdslider_adderror("Error INA219");
      Serial.println("Ошибка инициализации INA219");
    }
    // Read temperature sensor
    if(getInternalTemperatureData(&tsens_value)) {
      lcdslider_addparam("t:" + String(tsens_value) + "c");
    }
    else {
      lcdslider_adderror("Error Int Temp");
      Serial.println("Ошибка внутреннего датчика");
    }

    lcdslider_update();
    loopStage = true;
  } while(lcd_isButtonPress() && isHaveTimeDS(4000 + LCDTIMECOUNTERSLEEP * TIMEFACTOR_SMALL));
  //
  enableCore0WDT();

  if(getRemainTimeDS_left() > 0)
    ESP_SLEEP

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
  json += "\"INA219\":{ ";
  json += "\"voltage\":\"" + String(ina219data.voltage) + "\"}";
  json += "}";

  Serial.println (json);

  if(!jsonStringProcess(json))
    ESP_SLEEP

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
  ESP_SLEEP
}

void loop() {}

void exitProgram() {
  loraSleep();
  sleepAllSensors();
  lcd_goSleep();
  Serial.println("Time work: " + String(millis() + DEEPSLEEP_STARTDELAY));
  startDeepSleep();
}