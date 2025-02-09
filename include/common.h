#ifndef COMMON_H
#define COMMON_H

#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>

/* RTC MEMORY */
struct rtcsm {
  bool debugprint = true;
  bool encprint = false;
  bool clearkeysonboot = true;
  uint64_t rtctime_nextwakeup = 0;
  uint8_t lastsendloraid = 0; //Не используется
};
RTC_DATA_ATTR static struct rtcsm rtcspecmode;

//Replacing serial for data output mode
#define SerialP1 if (rtcspecmode.debugprint) Serial
#define SerialP2 if (rtcspecmode.encprint) Serial

#define I2C_FREQ  100000

#define ESP_SLEEP exitProgram ();

extern void exitProgram();

#endif /* COMMON_H */