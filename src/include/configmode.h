#ifndef CONFIGMODE_H
#define CONFIGMODE_H

/** 
  * @defgroup configmode Режим конфигурации
  * @brief Режим для изменения параметров конфигурации и работы устройства
  * @details Компонент для реализации режима разработчика с целью возможности изменения параметров конфигурации
  * устройства через взаимодействия с пользователем путем отправки им команд по Serial порту
  * @{
*/
#define CONFIGMODE_DEFPASS  "123pass" ///< Пароль по умолчанию для входа режим конфигурации
#define CONFIGMODE_TIME 60  ///< Время бездействия пользователя для выхода из режима конфигурации (в секундах)

/** Содержимое команды из Serial порта */
struct configmode_commands_t {
  String cmd;                                 /**< Ключ команды */
  void (*handler)(String params);             /**< Функция обработчик команды */
  String usage = "No info";                   /**< Опциональная информация о команде */ 
  //configmode_commands_t subcommands[] = NULL; /**< Дополнительные подкоманды */
}; 

/**
 * @brief Инициализация режима конфигурации
 * @details Функция устанавливает режимы INPUT на GPIO соответствующей кнопки. И в случае, если она нажата переводит устройство в данный режим
 */
void cfgmode_init();

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
 * @brief TODO
*/
bool cfgmode_call_command(String str, configmode_commands_t cfg_commands[], size_t size);

bool cfgmode_isauth();

void cfgmode_auth(String pass);

/*! @} */

#endif /* CONFIGMODE_H */