#include "src/include/common.h"   
#include "src/include/sensors.h"
//sensors
#include <INA219.h>
#include <GyverBME280.h>

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

void sleepAllSensors() {
  //need to sleep all sensors, bme280 no need
  ina219.sleep(true);
}