#ifndef DEEPSLEEP_H
#define DEEPSLEEP_H

//DeepSleep param
#define TIMEFACTOR_SMALL 1000
#define TIMEFACTOR_BIG 1000000L
#define TIME_DEEPSLEEP_DEFAULT 300  /* Time DeepSleep Default. Declaration in seconds. Used if not specified in fs */
#define DEEPSLEEP_STARTDELAY 40 

void wakeup_process(uint64_t &bitMask);

uint64_t getTimeDeepSleep();

uint64_t getBaseTimeDeepSleep();

bool checkWakeupGPIO(uint64_t bitMask, uint8_t pin);

void startDeepSleep();

uint64_t getRemainTimeDeepSleep();

int64_t getRemainTimeDS_left();

bool isHaveTimeDS(uint32_t msec);

#endif /* DEEPSLEEP_H */