// Copyright [2025] Name <email>

#ifndef INCLUDE_LORA_H_
#define INCLUDE_LORA_H_

/** 
  * @defgroup lora LoRA
  * @brief Компонент для работы LoRa
  * @details Компонент реализует работу с модулем LoRa и выполняет отправку сообщений
  * @{
*/


#define LORA_SF_DEFAULT 9       ///< Значение Spreading Factor по умолчанию для LoRa
#define LORA_BW_DEFAULT 125000  ///< Значение Band Width по умолчанию для LoRa
#define LORA_CR_DEFAULT 5       ///< Значение Coding Rate по умолчанию для LoRa

/**
 * @brief Структура, хранящая конфигурацию адресов LoRa
 */
typedef struct loraConfigAddress {
  uint8_t clientAddress = 0;  ///< Адрес LoRa клиента
  uint8_t gatewayAddress = 0; ///< Адрес LoRa шлюза
} loraConfigAddress;

/**
 * @brief Установить адрес Шлюза
 * @param[in] addr Адрес LoRa
 */
void loraSetGatewayAddress(uint8_t addr);

/**
 * @brief Установить адрес клиента (Этого устройства)
 * @param[in] addr Адрес LoRa
 */
void loraSetClientAddress(uint8_t addr);

/**
 * @brief Проверяет инициализацию драйвера модуля LoRa 
 * @return Истина если инициализирован, Ложь - если нет.
 */
bool loraIsInit();

/**
 * @brief Инициализирует драйвера модуля LoRa 
 * @return Истина если успешно, Ложь - если произошла ошибка.
 */
bool loraInit();

/**
 * @brief Пакует массив с ключами json в пакет LoRa и отправляет на сервер
 * @return Истина если пакет доставлен, Ложь - если произошла ошибка.
 */
bool loraSendKeys();

/**
 * @brief Пакует данные json из JsonDocument в пакет LoRa и отправляет на сервер
 * @return Истина если пакет доставлен, Ложь - если произошла ошибка.
 */
bool loraSendData();

/**
 * @brief Отправляет запрос очистки данных об устройстве на сервер  
 * @return Истина если пакет доставлен, Ложь - если произошла ошибка.
 */
bool loraSendClearData();

/**
 * @brief Выполняет полный цикл отправки сообщений LoRa в зависимости от текущего состояния
 * @details Данные для отправки предварительно должны быть загружены при помощи функции @ref jsonStringProcess.
 * Выполняет очистку данных, отправку ключей, данных json в зависимости от ответов сервера и текущего состояния 
 * клиента.  
 * @return Истина если все пакеты доставлены, Ложь - если произошла ошибка.
 */
bool loraSendFull();

/**
 * @brief Переводит модуль LoRa в спящий режим
 * @details Переводит модуль LoRa в спящий режим если драйвер модуля инициализирован, иначе
 * ничего не делает.
 */
void loraSleep();

/**
 * @brief Выполняет отправку пакета LoRa на сервер
 * @details Функция выполняет отправку пакета, хранящегося в буфере отправки, а значит все 
 * пакеты должны быть заранее подготовлены функциями @ref loraSendKeys, @ref loraSendData,
 * @ref loraSendClearData или @ref loraSendFull.
 * @return Истина если пакет доставлен, Ложь - если произошла ошибка.
 */
bool _loraSendAndGet();

/**
 * @brief Печатает в Serial порт содержимое буфера
 * @param[in] buf Указатель на буфер
 */
void _loraPrintBuf(uint8_t *buf);

/*! @} */

#endif // INCLUDE_LORA_H_