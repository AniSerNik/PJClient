#include <string>
#include <Regexp.h>
#include "src/include/common.h"
#include <RH_RF95.h> //for RH_RF95_MAX_MESSAGE_LEN
//
#include "src/include/json.h"

static JsonDocument jsonDoc;
static std::vector<std::string> jsonKeysCl;

extern uint8_t send_buf[RH_RF95_MAX_MESSAGE_LEN];
extern uint8_t recv_buf[RH_RF95_MAX_MESSAGE_LEN];

bool jsonStringProcess(String str) {
  DeserializationError err = deserializeJson(jsonDoc, str, DeserializationOption::NestingLimit(JSON_MAX_NESTEDOBJECT + 1));
  if (err) {
    Serial.println("deserializeJson() failed with code " + String(err.f_str()));
    return false;
  }
  return true;
}

/* Получение ключей */

void parseJsonKeys() {
  jsonKeysCl.clear();
  _getKeysEmbeddedObject(jsonDoc.as<JsonObject>());
}

void _getKeysEmbeddedObject(const JsonObject &embjson) {
  for (JsonPair kv : embjson) {
    if(std::find(jsonKeysCl.begin(), jsonKeysCl.end(), kv.key().c_str()) == jsonKeysCl.end())
      jsonKeysCl.push_back(kv.key().c_str());
    if (kv.value().is<JsonArray>())
      _getKeysEmbeddedArray(kv.value().as<JsonArray>());
    else if (kv.value().is<JsonObject>())
      _getKeysEmbeddedObject(kv.value().as<JsonObject>());
  }
}

void _getKeysEmbeddedArray(const JsonArray &embjson) {
  for (int i = 0; i < embjson.size(); i++) {
    if (embjson[i].is<JsonArray>())
      _getKeysEmbeddedArray(embjson[i].as<JsonArray>());
    else if (embjson[i].is<JsonObject>())
      _getKeysEmbeddedObject(embjson[i].as<JsonObject>());
  }
}

void encodeJsonKeys() {
  for (int i = 0; i < jsonKeysCl.size(); i++) {
    const char* key = jsonKeysCl[i].c_str();
    if(strlen(key) + 1 > sizeof(send_buf)) {
      Serial.println("Переполнение буфера при кодировке ключей");
      return;
    }
    strcpy((char*)&send_buf[send_buf[BYTE_COUNT]], key);
    send_buf[BYTE_COUNT] += strlen(key);
    strcpy((char*)&send_buf[send_buf[BYTE_COUNT]++], " ");
    SerialP1.println(String(i) + "\t" + String(key));
  }
}

/* Кодирование JSON */
void encodeJsonForSending() {
  _encodeJsonObject(jsonDoc.as<JsonObject>(), true);
}

void _encodeJsonObject(const JsonObject &embjson, bool isTopObj) {
  if(!isTopObj) {
    send_buf[send_buf[BYTE_COUNT]++] = SPECIFIC | JSON_OBJECT;
    SerialP2.println(F("object"));
  }
  for (JsonPair kv : embjson) {
    auto it = find(jsonKeysCl.begin(), jsonKeysCl.end(), kv.key().c_str()); 
    if (it == jsonKeysCl.end()) {
      Serial.print("error find key. Replace it before release version");
      return;
    }
    uint8_t idKey = it - jsonKeysCl.begin();
    SerialP2.print(kv.key().c_str());
    SerialP2.print(F(" : "));
    send_buf[send_buf[BYTE_COUNT]++] = idKey;
    if (kv.value().is<JsonArray>())
      _encodeJsonArray(kv.value().as<JsonArray>());
    else if (kv.value().is<JsonObject>())
      _encodeJsonObject(kv.value().as<JsonObject>());
    else
      _encodeJsonValue(kv.value().as<String>().c_str());
  }
  if(!isTopObj)
    send_buf[send_buf[BYTE_COUNT]++] = JSON_LEVEL_UP;
}

void _encodeJsonArray(const JsonArray &embjson) {
  SerialP2.println(F("array"));
  send_buf[send_buf[BYTE_COUNT]++] = SPECIFIC | JSON_ARRAY;
  for (int i = 0; i < embjson.size(); i++) {
    if (embjson[i].is<JsonArray>())
      _encodeJsonArray(embjson[i].as<JsonArray>());
    else if (embjson[i].is<JsonObject>())
      _encodeJsonObject(embjson[i].as<JsonObject>());
    else
      _encodeJsonValue(embjson[i].as<String>().c_str());
  }
  send_buf[send_buf[BYTE_COUNT]++] = JSON_LEVEL_UP;
}

void _encodeJsonValue(const char *val) {
  if(val == NULL)
    return;
  SerialP2.print(val);
  SerialP2.print(F(" is a "));
  uint8_t tag = 0x00;
  char value[JSON_MAX_LEN_FIELD];
  strlcpy(value, val, JSON_MAX_LEN_FIELD);
  MatchState ms;
  ms.Target(value);
  if(ms.Match("^%s*%-?%d+%.%d+%s*$") == REGEXP_MATCHED) {
    SerialP2.print(F("custom float "));
    tag |= CUSTOM_FLOAT;

    char integ[13], frac[16];
    int ret = sscanf(value, " %12[0-9-].%15[0-9]", integ, frac);
    if (ret != 2) {
      Serial.println(F("; input error, sending as string"));
      goto send_as_str;      
    }
    uint8_t decimal_places = strlen(frac);
    tag |= decimal_places;
    
    char integfrac[28];
    strcpy(integfrac, integ);
    strcat(integfrac, frac);      

    errno = 0;
    long equiv_int = strtol(integfrac, NULL, 10);
    if (errno != 0) {
      SerialP2.println(F("; conversion error, sending as string"));
      goto send_as_str;
    }
    
    if (INT8_MIN <= equiv_int && equiv_int <= INT8_MAX) {
      SerialP2.println(F("8 bit"));
      tag |= _8BIT;
      if(send_buf[BYTE_COUNT]+2 <= sizeof(send_buf)) {
        send_buf[send_buf[BYTE_COUNT]++] = tag;
        send_buf[send_buf[BYTE_COUNT]++] = equiv_int;
      }
    }
    else if (INT16_MIN <= equiv_int && equiv_int <= INT16_MAX) {
      SerialP2.println(F("16 bit"));
      tag |= _16BIT;
      if(send_buf[BYTE_COUNT]+3 <= sizeof(send_buf)) {
        send_buf[send_buf[BYTE_COUNT]++] = tag;
        send_buf[send_buf[BYTE_COUNT]++] = equiv_int & 0x00FF;
        send_buf[send_buf[BYTE_COUNT]++] = (equiv_int & 0xFF00) >> 8;
      }
    }
    else if (INT32_MIN <= equiv_int && equiv_int <= INT32_MAX) {
      SerialP2.println(F("32 bit"));
      tag |= _32BIT;
      if(send_buf[BYTE_COUNT]+5 <= sizeof(send_buf)) {
        send_buf[send_buf[BYTE_COUNT]++] = tag;
        for(int j = 0; j < 4; j++) {
          send_buf[send_buf[BYTE_COUNT]++] = equiv_int & 0x00FF;
          equiv_int = equiv_int >> 8;
        }
      }
    }
  }
  else if (ms.Match("^%s*%-?%d+%s*$") == REGEXP_MATCHED) {
    SerialP2.print(F("integer "));
    tag |= INTEGER;
    errno = 0;
    long num = strtol(value, NULL, 10);
    if (errno != 0) {
      SerialP2.println(F("; conversion error, sending as string"));
      goto send_as_str;
    }

    if (INT8_MIN <= num && num <= INT8_MAX) {
      SerialP2.println(F("8 bit"));
      tag |= _8BIT;
      if(send_buf[BYTE_COUNT]+2 <= sizeof(send_buf)) {
        send_buf[send_buf[BYTE_COUNT]++] = tag;
        send_buf[send_buf[BYTE_COUNT]++] = num;
      }
    }
    else if (INT16_MIN <= num && num <= INT16_MAX) {
      SerialP2.println(F("16 bit"));
      tag |= _16BIT;
      if(send_buf[BYTE_COUNT]+3 <= sizeof(send_buf)) {
        send_buf[send_buf[BYTE_COUNT]++] = tag;
        send_buf[send_buf[BYTE_COUNT]++] = num & 0x00FF;
        send_buf[send_buf[BYTE_COUNT]++] = (num & 0xFF00) >> 8;
      }
    }
    else if (INT32_MIN <= num && num <= INT32_MAX) {
      SerialP2.println(F("32 bit"));
      tag |= _32BIT;
      if(send_buf[BYTE_COUNT]+5 <= sizeof(send_buf)) {
        send_buf[send_buf[BYTE_COUNT]++] = tag;
        for(int j = 0; j < 4; j++) {
          send_buf[send_buf[BYTE_COUNT]++] = num & 0x00FF;
          num = num >> 8;
        }
      }
    }
  }
  else if (ms.Match("^%s*%x%x:%x%x:%x%x:%x%x:%x%x:%x%x%s*$") == REGEXP_MATCHED) {
    SerialP2.println(F("MAC"));
    tag |= SPECIFIC | MAC_ADDR;
    unsigned char mac[6];
    sscanf(value, " %hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    if(send_buf[BYTE_COUNT]+7 <= sizeof(send_buf)) {
      send_buf[send_buf[BYTE_COUNT]++] = tag;
      for(int i = 0; i < 6; i++)
        send_buf[send_buf[BYTE_COUNT]++] = mac[i];
    }   
  }
  else if(ms.Match("^%s*%d?%d?%d%.%d?%d?%d%.%d?%d?%d%.%d?%d?%d%s*$") == REGEXP_MATCHED) {
    SerialP2.println(F("IP"));
    tag |= SPECIFIC | IP_ADDR;
    unsigned char ip[4];
    sscanf(value, " %hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]);
    
    if(send_buf[BYTE_COUNT]+5 <= sizeof(send_buf)) {
      send_buf[send_buf[BYTE_COUNT]++] = tag;
      for(int i = 0; i < 4; i++)
        send_buf[send_buf[BYTE_COUNT]++] = ip[i];
    }   
  }
  else if(ms.Match("^%s*%d%d%d%d%-%d%d%-%d%d%s*$") == REGEXP_MATCHED) {
    SerialP2.println(F("date"));
    tag |= SPECIFIC | DATE_YYYYMMDD;
    unsigned int year;
    unsigned char month, day;
    sscanf(value, " %u-%hhu-%hhu", &year, &month, &day);
    if(send_buf[BYTE_COUNT]+5 <= sizeof(send_buf)) {
      send_buf[send_buf[BYTE_COUNT]++] = tag;
      send_buf[send_buf[BYTE_COUNT]++] = year & 0x00FF;     
      send_buf[send_buf[BYTE_COUNT]++] = (year & 0xFF00) >> 8;
      send_buf[send_buf[BYTE_COUNT]++] = month;
      send_buf[send_buf[BYTE_COUNT]++] = day;
    }
  }
  else if(ms.Match("^%s*%d%d:%d%d:%d%d%s*$") == REGEXP_MATCHED) {
    SerialP2.println(F("time"));
    tag |= SPECIFIC | TIME_HHMMSS;
    unsigned char hours, minutes, seconds;
    sscanf(value, " %hhu:%hhu:%hhu", &hours, &minutes, &seconds);
    if(send_buf[BYTE_COUNT]+4 <= sizeof(send_buf)) {
      send_buf[send_buf[BYTE_COUNT]++] = tag;
      send_buf[send_buf[BYTE_COUNT]++] = hours;
      send_buf[send_buf[BYTE_COUNT]++] = minutes;
      send_buf[send_buf[BYTE_COUNT]++] = seconds;
    }
  }
  else {
    send_as_str:
    SerialP2.println(F("string"));
    tag = STRING;
    uint8_t value_len = strlen(value) + 1;
    if (send_buf[BYTE_COUNT] + value_len < sizeof(send_buf)) {         
      send_buf[send_buf[BYTE_COUNT]++] = tag;
      strcpy((char*)&send_buf[send_buf[BYTE_COUNT]], value);
      send_buf[BYTE_COUNT] += value_len;        
    }
  }
}