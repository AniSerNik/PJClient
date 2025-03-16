// Copyright [2025] Name <email>

#ifndef INCLUDE_PINS_ASSIGNMENT_H_
#define INCLUDE_PINS_ASSIGNMENT_H_

#ifndef BIT
    #define BIT(x)  (1 << x)    /**< Вычисление битовой маски с установленым битом */
#endif

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
#define LCDPIN_BUTTON 2
#define CONFIGMODE_PIN 3

//ext1 interruption (for display and config mode buttons)
#define EXT1WAKEUP_MASK BIT(LCDPIN_BUTTON) | BIT(CONFIGMODE_PIN)

#endif // INCLUDE_PINS_ASSIGNMENT_H_