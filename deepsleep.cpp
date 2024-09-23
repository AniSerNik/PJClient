#include "driver/rtc_io.h"
#include "rtc.h"
//
#include "common.h"
#include "pins_assignment.h"
#include "fs.h"
//
#include "deepsleep.h"

static uint64_t remainTimeDeepSleep = 0;

void wakeup_process(uint64_t &bitMask) {
  bitMask = 0;
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
    if (rtcspecmode.rtctime_nextwakeup > (esp_rtc_get_time_us() - (millis() * TIMEFACTOR_SMALL))) {
      //We wakeup early
      remainTimeDeepSleep = rtcspecmode.rtctime_nextwakeup - (esp_rtc_get_time_us() - (millis() * TIMEFACTOR_SMALL));
      Serial.println("Устройство проснулось раньше на " + String(remainTimeDeepSleep / TIMEFACTOR_BIG) + "s");
    }
    bitMask = esp_sleep_get_ext1_wakeup_status();
  }
}

uint64_t getTimeDeepSleep() {
  uint64_t timeDeepSleepBase = getBaseTimeDeepSleep();
  int64_t timeDeepSleepFixed;
  if (remainTimeDeepSleep == 0)
    timeDeepSleepFixed = timeDeepSleepBase - ((millis() * TIMEFACTOR_SMALL) % timeDeepSleepBase);
  else {
    timeDeepSleepFixed = remainTimeDeepSleep - ((millis() * TIMEFACTOR_SMALL));
    if (timeDeepSleepFixed < 0) {
      uint16_t multiplier = (abs(timeDeepSleepFixed) / timeDeepSleepBase) + 1;
      timeDeepSleepFixed = timeDeepSleepFixed + (timeDeepSleepBase * multiplier);
    }
  }
  return timeDeepSleepFixed;
}

uint64_t getBaseTimeDeepSleep() {
  uint64_t timeDeepSleepBase = fsGetConfigParam<uint16_t>(FSCONFIGNAME_DEEPSLEEP);
  if (timeDeepSleepBase == 0)  //if not in fs
    timeDeepSleepBase = TIME_DEEPSLEEP_DEFAULT;
  timeDeepSleepBase = timeDeepSleepBase * TIMEFACTOR_BIG;  //s in ns
  return timeDeepSleepBase;
}

bool checkWakeupGPIO(uint64_t bitMask, uint8_t pin) {
  if(bitMask == 0)
    return false;
  return (((bitMask >> pin) & 0B00000001) == 1);
}

void startDeepSleep() {
  uint64_t timeDeepSleep = getTimeDeepSleep();
  esp_sleep_enable_timer_wakeup(timeDeepSleep);

  fsUnreg();
  
  esp_sleep_enable_ext1_wakeup(EXT1WAKEUP_MASK, ESP_EXT1_WAKEUP_ANY_LOW);

  rtcspecmode.rtctime_nextwakeup = esp_rtc_get_time_us() + (DEEPSLEEP_STARTDELAY * TIMEFACTOR_SMALL) + timeDeepSleep;
  delay(DEEPSLEEP_STARTDELAY);

  rtc_gpio_pullup_en((gpio_num_t)LCDPIN_BUTTON);
  rtc_gpio_pullup_en((gpio_num_t)DEBUGMODE_PIN);
  esp_deep_sleep_start();
}

uint64_t getRemainTimeDeepSleep() {
  return remainTimeDeepSleep;
}

int64_t getRemainTimeDS_left() {
  return remainTimeDeepSleep - (millis() * TIMEFACTOR_SMALL);
}

bool isHaveTimeDS(uint32_t msec) {
  uint64_t futuretime = (millis() + msec) * TIMEFACTOR_SMALL + 200;
  if(remainTimeDeepSleep == 0)
    return futuretime < getBaseTimeDeepSleep();
  else
    return futuretime < remainTimeDeepSleep;
}