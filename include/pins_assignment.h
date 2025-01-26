#ifndef PINS_ASSIGNMENT_H
#define PINS_ASSIGNMENT_H

//spi Param
#define SPI_CS 18
#define SPI_CLK 19
#define SPI_MOSI 20
#define SPI_MISO 21
//I2C Param
#define I2C_SDA 6
#define I2C_SCL 7
//lora param
#define LORA_DIO0 22
//lcd 
#define LCDPIN_TRANSISTOR 15
#define LCDPIN_BUTTON 2     // if change it - change EXT1WAKEUP_MASK!!!
#define CONFIGMODE_PIN 3    // if change it - change EXT1WAKEUP_MASK!!!

//ext1 interruption (for display and config mode buttons)
#define EXT1WAKEUP_MASK 0B00001100

#endif /* PINS_ASSIGNMENT_H */