// Copyright [2025] Name <email>

#include <esp_littlefs.h>
#include <common.h>
#include <filesystem.h>

//Параметры для создания файловой системы LittleFS
esp_vfs_littlefs_conf_t fsconf = {
  .base_path = "/lfs",
  .partition_label = "spiffs",
  .format_if_mount_failed = true,
  .dont_mount = false
};

bool fsInit() {
  SerialP1.println("Init LittleFS");
  esp_err_t ret = esp_vfs_littlefs_register(&fsconf);
  if (ret != ESP_OK) {
    if (ret == ESP_FAIL)
      Serial.println("Failed to mount or format filesystem");
    else if (ret == ESP_ERR_NOT_FOUND)
      Serial.println("Failed to find LittleFS partition");
    else
      Serial.println("Failed to initialize LittleFS (" + String(esp_err_to_name(ret)) + ")");
    return false;
  }
  return true;
}

void fsUnreg() {
  esp_vfs_littlefs_unregister(fsconf.partition_label);
}

void fsPrintSize() {
  size_t total = 0, used = 0;
  esp_err_t ret = esp_littlefs_info(fsconf.partition_label, &total, &used);
  if (ret != ESP_OK)
    Serial.println("Failed to get LittleFS partition information (" + String(esp_err_to_name(ret)) + ")");
  else
    Serial.println("Partition size: total: " + String(total) + ", used: " + String(used));
}

bool fsIsHaveConfigFile(const char *path) {
  struct stat st;
  if (stat(path, &st) == 0)
    return true;
  return false;
}

bool fsOpenConfigFile(JsonDocument *jsonDoc, const char *path) {
  if(!fsIsHaveConfigFile(path)) {
    fsClearConfigFile(path);
  }
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    Serial.println("Ошибка открытия конфиг файла для чтения. Путь: " + String(path));
    return false;
  }
  char str [fsMaxConfigSizeFile];
  fread(str, sizeof(char), sizeof(str), f);
  jsonDoc->clear();
  DeserializationError err = deserializeJson(*jsonDoc, str);
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.f_str());
    fclose(f);
    return false;
  }
  fclose(f);
  return true;
}

bool fsSaveConfigFile(JsonDocument *jsonDoc, const char *path) {
  FILE *f = fopen(path, "w");
  if (f == NULL) {
    Serial.println("Ошибка открытия конфиг файла для записи. Путь: " + String(path));
    return false;
  }
  size_t jsonBytesSize = measureJson(*jsonDoc) + 1;
  char str[jsonBytesSize] = {0};
  size_t countBytes = serializeJson(*jsonDoc, str, jsonBytesSize);
  if(!jsonDoc->isNull() && countBytes == 0) {
    Serial.println("Ошибка записи в конфиг файл, serialize вернул 0 байт при непустом JsonDoc");
    fclose(f);
    return false;
  }
  size_t writeBytes = fwrite(str, sizeof(char), countBytes, f);
  if(writeBytes != countBytes) {
    Serial.println("Ошибка записи в конфиг файл, записаны не все байты. Записано " + String(writeBytes) + "б; Необходимо " + String(countBytes) + "б" + ". Путь: " + String(path));
    fclose(f);
    return false;
  }
  fclose(f);
  return true;
}

bool fsClearConfigFile(const char *path) {
  JsonDocument jsonDoc;
  return fsSaveConfigFile(&jsonDoc, path);
}

void fsPrintConfigFile() {
  Serial.println("Содержимое конфиг файла:");
  FILE *f = fopen(FSCONFIGFILE, "r");
  if (f == NULL) {
    Serial.println("Ошибка открытия конфиг файла для чтения");
    return;
  }
  char str[fsMaxConfigSizeFile] = {0};
  fread(str, sizeof(char), sizeof(str), f);
  Serial.println(str);
  fclose(f);
}