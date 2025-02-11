#include "common.h"  
#include "sensors.h"
//sensors
#include <INA219.h>
#include <GyverBME280.h>
#include <driver/temperature_sensor.h>

//Инициализация объектов для работы с сенсорами
static INA219 ina219((uint8_t)INA219_ADDRESSI2C);
static GyverBME280 bme280;

bool getBME280Data(BME280SensorData* SData) {
  bme280.setMode(FORCED_MODE);
  bme280.setStandbyTime(STANDBY_1000MS);
  if (!bme280.begin((uint8_t)BME280_ADDRESSI2C))
    return false;

  bme280.oneMeasurement();   
  while (bme280.isMeasuring());
  SData->temperature = bme280.readTemperature();
  SData->humidity = bme280.readHumidity();
  SData->pressure = pressureToMmHg(bme280.readPressure());
  return true;
}

bool getINA219Data(INA219SensorData* SData) {
  if (!ina219.begin())
    return false;
    
  ina219.sleep(false);
  ina219.setResolution(INA219_VBUS, INA219_RES_12BIT_X128);
  delay(10);
  SData->voltage = ina219.getVoltage();
  SData->current = ina219.getCurrent();
  SData->power = ina219.getPower();
  return true;
}

bool getInternalTemperatureData(float *tsens_value) {
  esp_err_t errorCode = ESP_OK;
  temperature_sensor_handle_t temp_sensor = NULL;
  temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-30, 50);
  int cnt = 2;
  ESP_ERROR_CHECK_WITHOUT_ABORT(errorCode = temperature_sensor_install(&temp_sensor_config, &temp_sensor));
  if (errorCode != ESP_OK)
    goto fail;
  ESP_ERROR_CHECK_WITHOUT_ABORT(errorCode = temperature_sensor_enable(temp_sensor));
  if (errorCode != ESP_OK)
    goto fail;
  while (cnt--) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(errorCode = temperature_sensor_get_celsius(temp_sensor, tsens_value));
    if (errorCode != ESP_OK)
      goto fail;
    delay(20);
  }
fail: 
  ESP_ERROR_CHECK_WITHOUT_ABORT(temperature_sensor_disable(temp_sensor));
  ESP_ERROR_CHECK_WITHOUT_ABORT(temperature_sensor_uninstall(temp_sensor));
  if (errorCode == ESP_OK)
    return true;
  return false;
}

void sleepAllSensors() {
  //need to sleep all sensors, bme280 no need
  ina219.sleep(true);
}