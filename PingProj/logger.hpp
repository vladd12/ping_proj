#ifndef LOGGER_H
#define LOGGER_H

#include "main.hpp"

using std::locale;

// Типы сообщений, передаваемые логгеру
#define INIT_MESSAGE	0
#define INFO_MESSAGE	1
#define ERROR_MESSAGE	2
#define CLOSE_MESSAGE	3

int InitLogger();
int WriteMessage(int, int, UINT);
string GetSystemTime();

#endif
