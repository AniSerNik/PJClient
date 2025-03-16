// Copyright [2025] Name <email>

#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

/** 
  * @defgroup common Стандартные настройки
  * @brief Задает стандартные настройки для использования в любом компоненте прошивки
  * @{
*/

#include <platform.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <esp_task_wdt.h>

/**
  * @brief RTC MEMORY
  * @details Структура для хранения данных в памяти RTC. Эта память не очищается в deepsleep, но очищается при 
  * отключении ESP32 от питания. Используется для сохранения части состояний между выходами из deepsleep
  */
struct rtcsm {
  bool debugprint = true;           ///< Режим отладочной информации в Serial порт
  bool encprint = false;            ///< Режим информации о кодировании LoRa пакетов в Serial порт
  bool clearkeysonboot = true;      ///< Флаг запроса очистки данных на сервере о клиенте. Сбрасывается после очистки при первом запуске
  uint64_t rtctime_nextwakeup = 0;  ///< Хранит время (От первого запуска ESP32) следующего запланированого выхода устройства из DeepSleep
  uint8_t lastsendloraid = 0;       ///< Не используется. Предполагалось увеличения счетчиков пакета для LoRa
};

RTC_DATA_ATTR static struct rtcsm rtcspecmode;

#define SEC_TO_MS(sec) ((sec)*1000)               ///< Перевод секунд в миллисекунды

/* Секция с настройками WATCHDOG */
#define WDT_TIMEOUT 5                             ///< Таймаут для WatchDog после которого он перезапускает плату

static esp_task_wdt_config_t twdt_config = {
  .timeout_ms = SEC_TO_MS(WDT_TIMEOUT),           ///< Таймаут для WatchDog, после которого он перезапускает плату
  .idle_core_mask = (1 << configNUM_CORES) - 1,   ///< Битовая маска на которых WDT работает. По умолчанию - на всех.
  .trigger_panic = true,                          ///< Флаг для перезагрузки ESP32 при срабатывании WDT
};

//Replacing serial for data output mode
#define SerialP1 if (rtcspecmode.debugprint) Serial
#define SerialP2 if (rtcspecmode.encprint) Serial

#define I2C_FREQ  100000              ///< Частота I2C шины

extern void exitProgram();

#define ESP_SLEEP exitProgram ();     ///< Макрос для перевода ESP32 в deepsleep незамедлительно

#endif // INCLUDE_COMMON_H_