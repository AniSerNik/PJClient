#include "driver/rtc_io.h"
#include "rtc.h"
//
#include "common.h"
#include "pins_assignment.h"
#include "filesystem.h"
//
#include "deepsleep.h"

static uint64_t remainTimeDeepSleep = 0;

static String _getWakeupCauseText (esp_sleep_wakeup_cause_t reason);

void wakeup_process(uint64_t &bitMask) {
  Serial.println("Причина пробуждения - " + _getWakeupCauseText(esp_sleep_get_wakeup_cause()));
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
      timeDeepSleepFixed += (timeDeepSleepBase * multiplier);
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
  return (((bitMask >> pin) & 0B01) == 1);
}

void startDeepSleep() {
  uint64_t timeDeepSleep = getTimeDeepSleep();
  esp_sleep_enable_timer_wakeup(timeDeepSleep);

  fsUnreg();
  
  esp_sleep_enable_ext1_wakeup(EXT1WAKEUP_MASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  rtcspecmode.rtctime_nextwakeup = esp_rtc_get_time_us() + (DEEPSLEEP_STARTDELAY * TIMEFACTOR_SMALL) + timeDeepSleep;
  delay(DEEPSLEEP_STARTDELAY);

  // Заземляем все пины с прерываниями
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  rtc_gpio_pulldown_en((gpio_num_t)LCDPIN_BUTTON);
  rtc_gpio_pullup_dis((gpio_num_t)LCDPIN_BUTTON);
  rtc_gpio_pulldown_en((gpio_num_t)CONFIGMODE_PIN);
  rtc_gpio_pullup_dis((gpio_num_t)CONFIGMODE_PIN);

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

static String _getWakeupCauseText (esp_sleep_wakeup_cause_t reason) {
  String text;
  switch (reason) {
    case ESP_SLEEP_WAKEUP_UNDEFINED:
      text = "Неопределено";
      break;
    case ESP_SLEEP_WAKEUP_EXT0:
      text = "внешний сигнал EXT0";
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      text = "внешний сигнал EXT1 (Кнопка?)";
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      text = "таймер";
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      text = "touchpad";
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      text = "программой ULP";
      break;
    case ESP_SLEEP_WAKEUP_COCPU:
      text = "COCPU int";
      break;
    case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
      text = "сбой COCPU";
      break;
    default:
      text = "Unclown";
      break;
  }
  return text;
}