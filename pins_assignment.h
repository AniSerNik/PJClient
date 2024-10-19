#ifndef PINS_ASSIGNMENT_H
#define PINS_ASSIGNMENT_H

//spi Param
#define SPI_CS LORA_CS
#define SPI_CLK LORA_SCK
#define SPI_MOSI LORA_MOSI
#define SPI_MISO LORA_MISO
//I2C Param
#define I2C_SDA SDA
#define I2C_SCL SCL
//lora param
//#define LORA_DIO0
//lcd 
#define LCDPIN_TRANSISTOR 39
#define LCDPIN_BUTTON 34      // if change it - change EXT1WAKEUP_MASK!!!
#define DEBUGMODE_PIN 40      // if change it - change EXT1WAKEUP_MASK!!!

//ext1 interruption (for display and debug mode buttons)
#define EXT1WAKEUP_MASK 0B00001100

#endif /* PINS_ASSIGNMENT_H */