// Copyright [2025] Name <email>

#ifndef INCLUDE_JSON_H_
#define INCLUDE_JSON_H_

/** 
  * @defgroup Json 
  * @brief 
  * @details 
  * @{
*/

#include <ArduinoJson.h>

//json Param
#define JSON_MAX_LEN_FIELD 32
#define JSON_MAX_NESTEDOBJECT 16

/* code for client / server - They must be the same
  Used for the transmission protocol  */
#define COMMAND 0
#define BYTE_COUNT 1
#define START_PAYLOAD 2

#define LORA_NOP 0
#define REGISTRATION 1
#define DATA 2
#define CLEAR_INFO 3

#define REPLY_TRUE 1
#define REPLY_FALSE 0

#define INTEGER 0x00        //00000000
#define CUSTOM_FLOAT 0x40   //01000000
#define STRING 0x80         //10000000
#define SPECIFIC 0xC0       //11000000
#define _8BIT 0x00          //00000000
#define _16BIT 0x10         //00010000
#define _32BIT 0x20         //00100000
#define IP_ADDR 0x01        //00000001
#define MAC_ADDR 0x02       //00000010
#define DATE_YYYYMMDD 0x03  //00000011
#define TIME_HHMMSS 0x04    //00000100
#define JSON_OBJECT 0x05    //00000101
#define JSON_ARRAY 0x06     //00000110
#define JSON_LEVEL_UP 0xFF  //11111111
/* end code for client / server */

bool jsonStringProcess(String str);

/* Получение ключей */

void parseJsonKeys();

void _getKeysEmbeddedObject(const JsonObject &embjson);

void _getKeysEmbeddedArray(const JsonArray &embjson);

void encodeJsonKeys();

/* Кодирование JSON */

void encodeJsonForSending();

void _encodeJsonObject(const JsonObject &embjson, bool isTopObj = false);

void _encodeJsonArray(const JsonArray &embjson);

void _encodeJsonValue(const char *val);

/*! @} */

#endif // INCLUDE_JSON_H_