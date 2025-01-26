#ifndef SENSORS_H
#define SENSORS_H

/** 
  * @defgroup sensors Сенсоры
  * @brief Реализация работы с сенсорами
  * @details В данном компоненте реализуются возможности для работы с сенсорами, включая считывания, обработку значений и перевод их в сон
  * @{
*/

#define SENSORSVALUE_DEFAULT 255  ///< Значение по умолчанию, которое возвращается при ошибке чтения с сенсора

#define INA219_ADDRESSI2C 0x40    ///< Адрес I2C для INA219
#define BME280_ADDRESSI2C 0x76    ///< Адрес I2C для BME280

/**
 * @brief Структура содержит в себе данные с датчика BME280 
 */
typedef struct BME280SensorData {
  float temperature = SENSORSVALUE_DEFAULT; ///< Температура
  float humidity = SENSORSVALUE_DEFAULT;    ///< Влажность
  float pressure = SENSORSVALUE_DEFAULT;    ///< Давление
} BME280SensorData;

/**
 * @brief Структура содержит в себе данные с датчика INA219 
 */
typedef struct INA219SensorData {
  float voltage = SENSORSVALUE_DEFAULT;     ///< Напряжение
  float current = SENSORSVALUE_DEFAULT;     ///< Сила тока
  float power = SENSORSVALUE_DEFAULT;       ///< Мощность
} INA219SensorData;

/**
 * @brief Функция для работы с датчиком BME280
 * @param[in, out] SData Структура используется для возврата значений с датчика
 * @return Истина - если удалось считать данные, ложь - если произошла ошибка
 */
bool getBME280Data(BME280SensorData* SData);

/**
 * @brief Функция для работы с датчиком INA219
 * @param[in, out] SData Структура используется для возврата значений с датчика
 * @return Истина - если удалось считать данные, ложь - если произошла ошибка
 */
bool getINA219Data(INA219SensorData* SData);

/**
 * @brief Перевод всех датчиков в режим сна
 * @details При добавлении нового датчика - необходимо изменения тела функции
 */
void sleepAllSensors();

/*! @} */

#endif  /* SENSORS_H */