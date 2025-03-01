#ifndef DEEPSLEEP_H
#define DEEPSLEEP_H

/** 
  * @defgroup DeepSleep 
  * @brief 
  * @details 
  * @{
*/
//DeepSleep param
#define TIMEFACTOR_SMALL 1000       ///< Макрос для перевода из нС в мС, мС в С и наоборот
#define TIMEFACTOR_BIG 1000000L     ///< Макрос для перевода из нС в С и наоборот
#define TIME_DEEPSLEEP_DEFAULT 300  ///< Время deepsleep по уполчанию. Используется если не задано иное в файле конфигурации. Указывается в секундах. */
#define DEEPSLEEP_STARTDELAY 40     ///< Задержка в мС перед переходом в deepsleep

/**
 * @brief Обрабатывает причину выхода ESP32 из deepsleep
 * @details Возвращает битовую маску @ref bitMask, если выход произошел по причине
 * нажатия на них (EXT1)
 * @param[out] bitMask Битовая маска нажатых кнопок
 */
void wakeup_process(uint64_t &bitMask);

/**
 * @brief Возвращает скорректированное время deepSleep
 * @details Корректирует время с учетом времени работы прошивки ESP32 для
 * сохранения заданного интервала.
 * @return Скоректированное время deepSleep
 */
uint64_t getTimeDeepSleep();

/**
 * @brief Возвращает базовое время deepSleep
 * @details Берет время из файла конфигурации или по умолчанию, если не задано
 * @return Время deepSleep
 */
uint64_t getBaseTimeDeepSleep();

/**
 * @brief Проверяет статус кнопки из битовой маски GPIO
 * @param[in] bitMask Битовая маска GPIO
 * @param[in] pin Конкретный пин
 * @return Статус конкретного пина GPIO
 */
bool checkWakeupGPIO(uint64_t bitMask, uint8_t pin);

/**
 * @brief Запускает deepsleep
 * @details Считает время и устанавливает необходимую конфигурацию DeepSleep и запускает его. 
 */
void startDeepSleep();

/**
 * @brief Возвращает оставшееся время до следующего предпологаемого выхода из DeepSleep (Передачи по LoRa)
 * @return Время в ns
 */
int64_t getRemainTimeDeepSleep();

/**
 * @brief Проверяет останется ли через N мс (@c msec) время до следующего предпологаемого выхода из DeepSleep (Передачи по LoRa)
 * @param[in] Требуемое время в мс
 * @return True or False
 */
bool isHaveTimeDS(uint32_t msec);

/*! @} */

#endif /* DEEPSLEEP_H */