// Copyright [2025] Name <email>

#ifndef INCLUDE_PLATFORM_H_
#define INCLUDE_PLATFORM_H_

/* 
  Режим работы дисплея в активное время микроконтроллера.
  Если 1, то дисплей включается и выключается транзистором.
  Если 0, то транзистор дисплея всегда работает, 
  но дисплей может уходить в ligth sleep 
*/
#define ENV_TRANSISTORMODE 0

#define ENV_TEMPSENSORCALIBRATE 8 ///< Значение для калибровки внутреннего термодатчика ESP32. Прибавляется к значению

#endif  // INCLUDE_PLATFORM_H_