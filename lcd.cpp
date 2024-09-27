#include <LiquidCrystal_I2C.h>
#include "src/include/common.h"
#include "src/include/pins_assignment.h"
#include "src/include/deepsleep.h"
#include <vector>
#include "src/include/lcd.h"

static LiquidCrystal_I2C lcd((uint8_t)LCD_ADDRESSI2C, 20, 4);

static std::vector<String> lcdslider_param;
static std::vector<String> lcdslider_error;

static bool lcd_active = false;

void lcd_init() {
  pinMode(LCDPIN_TRANSISTOR, OUTPUT);
  pinMode(LCDPIN_BUTTON, INPUT_PULLDOWN);
  if(lcd_isButtonPress() || LCD_ALWAYSENABLED)
    lcd_on();
}

void lcd_printstatus(String str) {
  String clear;
  for(int i = 0; i < LCDSTATUS_MAXCHAR; i++)
    clear += ' ';
  lcd_print(clear, 0, 0);
  if(str.length() > LCDSTATUS_MAXCHAR)
    str.remove(LCDSTATUS_MAXCHAR, str.length());
  lcd_print(str, 0, 0);
}

void lcd_clearLine(int row) {
  String clear;
  for(int i = 0; i < 20; i++)
    clear += ' ';
  lcd_print(clear, 0, row);
}

void lcd_print(String str, int col = -1, int row = -1) {
  if(!lcd_active)
    return;
  lcd.setCursor(col, row);
  lcd.print(str);
}

void lcd_on() {
  digitalWrite(LCDPIN_TRANSISTOR, HIGH);
  delay(25);
  lcd.init();
  lcd.backlight();
  lcd_active = true;
}

void lcd_off() {
  lcd.noBacklight();
  digitalWrite(LCDPIN_TRANSISTOR, LOW);
  lcd_active = false;
}

bool lcd_isActive() {
  return lcd_active;
}

bool lcd_isButtonPress() {
  return (digitalRead(LCDPIN_BUTTON) == HIGH);
}

void lcd_goSleep() {
  if(!lcd_isActive())
    return;
    
lcd_startsleep:
  if(lcd_isButtonPress()) {
    lcd_clearLine(3);
    lcd_print("Wait button release", 0, 3);
    while(lcd_isButtonPress() && isHaveTimeDS(2000 + LCDTIMECOUNTERSLEEP * TIMEFACTOR_SMALL))
      delay(1000);
  }
  if(isHaveTimeDS(LCDTIMECOUNTERSLEEP * TIMEFACTOR_SMALL)) {
    lcd_clearLine(3);
    bool isNotForced = isHaveTimeDS(1000 + LCDTIMECOUNTERSLEEP * TIMEFACTOR_SMALL);
    if(isNotForced)
      lcd_print("Sleep in...", 0, 3);
    else
      lcd_print("Sleep forced...", 0, 3);
    for(int i = LCDTIMECOUNTERSLEEP; i > 0; i--) {
      lcd_print(String(i), 19, 3);
      if(lcd_isButtonPress() && isNotForced)
        goto lcd_startsleep;
      delay(1000);
    }
  }
}

void lcdslider_clear() {
  lcdslider_param.clear();
  lcdslider_error.clear();
  lcdslider_update(true);
}

void lcdslider_addparam(String str) {
  lcdslider_param.push_back(str);
}

void lcdslider_adderror(String error) {
  lcdslider_error.push_back(error);
}

void lcdslider_update(bool clear) {
  if(!lcd_isActive())
    return;
  static uint8_t lcdslider_currentpage = 0, showerror = 0, showparam = 0;
  uint8_t curRow = 0;
  if(clear || lcdslider_currentpage == LCDSLIDER_MAXCOUNTPAGE || (showerror == (lcdslider_error.size() - 1) && showparam == (lcdslider_param.size() - 1))) {
    lcdslider_currentpage = 0;
    showerror = 0;
    showparam = 0;
    if(clear)
      return;
  }
  for(int i = LCDSLIDER_STARTROW; i < (LCDSLIDER_STARTROW + LCDSLIDER_MAXCOUNTROW); i++)
    lcd_clearLine(i);
  while(showerror < lcdslider_error.size()) {
    lcd_print(lcdslider_error[showerror], 0, LCDSLIDER_STARTROW + curRow);
    curRow++;
    showerror++;
    if(curRow == LCDSLIDER_MAXCOUNTROW) {
      lcdslider_currentpage++;
      return;
    }
  }
  while(showparam < lcdslider_param.size()) {
    lcd_print(lcdslider_param[showparam], 0 + (showparam % 2 * 10), LCDSLIDER_STARTROW + curRow);
    curRow += showparam % 2;
    showparam++;
    if(curRow == LCDSLIDER_MAXCOUNTROW) {
      lcdslider_currentpage++;
      return;
    }
  }
  lcdslider_currentpage++;
}