// Copyright [2025] Name <email>

#ifndef INCLUDE_FILESYSTEM_H_
#define INCLUDE_FILESYSTEM_H_

/** 
  * @defgroup fs Файловая система
  * @brief Компонент для работы с файловой системой и файлом конфигурации
  * @details Данный компонент системы позволяет работать с файловой системой LittleFS, создавая ее, добавляя, изменяя, читая и удаляя в ней файлы, а также содержит
  * функционал для взаимодействия с файлом конфигурации, который представляет собой Json. Работа с файловой системой осуществляется при помощи VFS (Виртуальная файловая система)
  * ESP32, а с файлом конфигурации при помощи библиотеки ArduinoJson (Устарело - путем хранения параметров в глобальном Json Document и взаимодействия с ним).
  * @{
*/

#include <ArduinoJson.h>

#define FSCONFIGFILE "/lfs/config.json" ///< Путь хранения конфигурационного файла
#define FSSECURECFGFILE "/lfs/config_secure.json" ///< Путь хранения защищенного конфигурационного файла
#define fsMaxConfigSizeFile 1024        ///< Максимальный размер конфигурационного файла (При открытии)

#define FSCONFIGNAME_ID "id"            ///< ID устройства (Ключ json для хранения в файле конфигурации)
#define FSCONFIGNAME_GATEWAY "gate"     ///< ID шлюза (Ключ json для хранения в файле конфигурации)
#define FSCONFIGNAME_DEEPSLEEP "sleep"  ///< Время DeelSleep (в сек) (Ключ json для хранения в файле конфигурации)
#define FSCONFIGNAME_LORABW "lorabw"    ///< Параметр lora - Band Width (Ключ json для хранения в файле конфигурации)
#define FSCONFIGNAME_LORACR "loracr"    ///< Параметр lora - Coding Rate (Ключ json для хранения в файле конфигурации)
#define FSCONFIGNAME_LORASF "lorasf"    ///< Параметр lora - Spreading Factor (Ключ json для хранения в файле конфигурации)
#define FSCONFIGNAME_CFGMODEPASS "cfgmodepass"    ///< Пароль для конфигурационного режима (Ключ json для хранения в файле конфигурации)

#define FSSECURECFGNAME_CGFMODE_PASS "cfgmode_pass"


/**
  * @brief Инициализация файловой системы
  * @details Инициализация LittleFS в рамках виртуальной файловой системы esp32. 
  * Регистрация в таком плане позволяет автоматически вызывать драйвер файловой системы при обращении к закрепленной за ней директории.
  * @return Функция возвращает статус регистрации файловой системы
*/
bool fsInit();

/**
  * @brief Деинициализация файловой системы
  * @details Удаляет привязку LittleFS к VFS. Делает невозможным работу с файловой системой до следующей инициализациии
*/
void fsUnreg();

/**
  * @brief Печать в Serial порт размер файловой системы
  * @details Печатается суммарно занимаемый объем + Используемый в данный момент
*/
void fsPrintSize();

/**
  * @brief Проверка существования файла конфигурации
  * @return Булево значение в зависимости от наличия файла.
*/
bool fsIsHaveConfigFile(const char *path);

/**
  * @brief Открытие файла конфигурации
  * @details Открывает файл конфигурации и записывает его в специальный Json Document
  * @return Истина, если удалось открыть, ложь - если нет.
*/
bool fsOpenConfigFile(JsonDocument *jsonDoc, const char *path);

/**
  * @brief Сохранение файла конфигурации
  * @details Приводит Json Document к строке и сохраняет ее в файл конфигурации
  * @return Истина, если удалось записать, ложь - если нет.
*/
bool fsSaveConfigFile(JsonDocument *jsonDoc, const char *path);

/**
  * @brief Очистка файла конфигурации
  * @details Очищает Json Document и файл конфигурации, а также сбрасывает переменную, указывающую на статус чтения файла. 
  * @return Истина, если удалось очистить файл, ложь - если нет. JsonDocument очищается в любом случае
*/
bool fsClearConfigFile(const char *path);

/**
  * @brief Печать файла конфигурации
  * @details Открывает файл конфигурации и печатает его в Serial порт
*/
void fsPrintConfigFile();

/**
  * @brief Установить параметр в файл конфигурации
  * @details Функция устанавливает значение заданного параметра в Json Document и сохраняет его в файловую систему
  * @param[in] name Имя (Ключ) параметра
  * @param[in] value Значение параметра 
  * @return Истина - если удалось сохранить изменения, ложь - если возникла ошибка
*/
template<typename Type>
bool fsSetConfigParam(String name, Type value) {
  JsonDocument jsonDoc;
  if(!fsOpenConfigFile(&jsonDoc, FSCONFIGFILE))
    return NULL;
  jsonDoc[name] = value;
  return fsSaveConfigFile(&jsonDoc, FSCONFIGFILE);
}

/**
  * @brief Получение параметра из файла конфигурации
  * @details Функция возвращает значение запрошенного параметра из Json Document. Функция сама открывает и читает файл конфигурации при необходимости.
  * @param[in] name Имя (Ключ) параметра
  * @return Значение параметра или NULL при ошибке 
*/
template<typename Type>
Type fsGetConfigParam(String name) {
  JsonDocument jsonDoc;
  if(!fsOpenConfigFile(&jsonDoc, FSCONFIGFILE))
    return (Type) NULL;
  if (jsonDoc[name].isNull())
    return (Type) NULL;
  return jsonDoc[name];
}

/*! @} */
#endif  // INCLUDE_FILESYSTEM_H_