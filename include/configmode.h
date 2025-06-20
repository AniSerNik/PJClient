// Copyright [2025] Name <email>

#ifndef INCLUDE_CONFIGMODE_H_
#define INCLUDE_CONFIGMODE_H_

/** 
  * @defgroup configmode Режим конфигурации
  * @brief Режим для изменения параметров конфигурации и работы устройства
  * @details Компонент для реализации режима разработчика с целью возможности изменения параметров конфигурации
  * устройства через взаимодействия с пользователем путем отправки им команд по Serial порту
  * @{
*/
#define CONFIGMODE_DEFPASS  "123pass" ///< Пароль по умолчанию для входа режим конфигурации
#define CONFIGMODE_TIME 60  ///< Время бездействия пользователя для выхода из режима конфигурации (в секундах)

/** 
 * @brief Структура с информацией о команде
 */
struct configmode_commands_t {
  String cmd;                                 /**< Ключ команды */
  void (*handler)(String params);             /**< Функция обработчик команды */
}; 

/**
 * @brief Возможные состояния вызова команды
 */
enum cfgmode_call_command_status {
  CFG_CALLCOMMAND_SUCCESS = 0,    ///< Команда выполнена успешно
  CFG_CALLCOMMAND_NOTFOUND = -1,  ///< Команда не найдена
};

/**
 * @brief Инициализация режима конфигурации
 * @details Функция устанавливает режимы INPUT на GPIO соответствующей кнопки. И в случае, если она нажата переводит устройство в данный режим
 */
void cfgmode_init();

/**
 * @brief Проверка кнопки режима конфигурации
 * @details Если нажата кнопка, функция переводит устройство в режима конфигурации
 */
void cfgmode_checkbutton();

/**
 * @brief Включение режима конфигурации
 * @details Включает дисплей, переводит устройство в режим конфигурации и начинает слушать команды по Serial порту. В случае, если их долго нет - выключает устройство
 */
void cfgmode_enable();

/**
 * @brief Обработка команды из Serial порта
 * @details Обрабатывает и исполняет команду, если она верна. 
 * @param[in] command Строка с анализируемой командой
 */
void cfgmode_processcommand(String command);

/**
 * @brief Осуществляет поиск команды и ее вызов
 * @details Выполняет поиск команды в массиве структур с командами и в случае, если находит
 * вызывает функцию обработчик команды, передавая ей параметры команды.
 * 
 * @param[in] str Строка с командой из Serial порта
 * @param[in] cfg_commands Массив структур с командами
 * @param[in] size Размер массива структур с командами
 * @return Состояние вызова команды @ref cfgmode_call_command_status
*/
int cfgmode_call_command(String str, configmode_commands_t cfg_commands[], size_t size);

/**
 * @brief Возвращает состояние авторизации в конфигурационном режиме
 * @return Истина, если пользователь авторизовался, Ложь - если нет
*/
bool cfgmode_isauth();

/**
 * @brief Проверяет пароль и выполняет авторизацию в конфигурационном режиме
 * @param[in] pass Введенный пароль для авторизации 
*/
void cfgmode_auth(String pass);

/*! @} */

#endif // INCLUDE_CONFIGMODE_H_