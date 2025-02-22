#ifndef COMMON_H
#define COMMON_H

#include <platform.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include "esp_task_wdt.h"

/* RTC MEMORY */
struct rtcsm {
  bool debugprint = true;
  bool encprint = false;
  bool clearkeysonboot = true;
  uint64_t rtctime_nextwakeup = 0;
  uint8_t lastsendloraid = 0; //Не используется
};
RTC_DATA_ATTR static struct rtcsm rtcspecmode;

#define SEC_TO_MS(sec) ((sec)*1000)

/* WATCHDOG */
#define WDT_TIMEOUT 5

static esp_task_wdt_config_t twdt_config = {
  .timeout_ms = SEC_TO_MS(WDT_TIMEOUT),
  .idle_core_mask = (1 << configNUM_CORES) - 1,    // Bitmask of all cores,
  .trigger_panic = true,
};

//Replacing serial for data output mode
#define SerialP1 if (rtcspecmode.debugprint) Serial
#define SerialP2 if (rtcspecmode.encprint) Serial

#define I2C_FREQ  100000

#define ESP_SLEEP exitProgram ();

extern void exitProgram();

#endif /* COMMON_H */