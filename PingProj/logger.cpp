#include "logger.hpp"
#include <Windows.h>
#include <exception>

wFile LogFile;

// Запуск логгера
int InitLogger() {
	try {
		locale defaultLocale(dLocale);
		LogFile.imbue(defaultLocale);
		LogFile.open("log.txt", std::ios::app);
		if (WriteMessage(INIT_MESSAGE, NULL) == FUNC_ERROR) {
			DebugInfo(CurFunc);
			return FUNC_ERROR;
		}
		else return FUNC_SUCCESS;
	}
	// Обращаемся к обработчику ошибок
	catch (std::exception& e) {
		DebugInfo(CurFunc);
		LogFile.close();
		HandleError(INIT_MESSAGE, NULL, e.what());
		return FUNC_ERROR;
	}
}

// Запись входящего сообщения с помощью логгера
int WriteMessage(int type, string* msg) {
	try {
		string time;
		time = GetSystemTime();
		switch (type) {
		case INIT_MESSAGE:
			LogFile << TEXT('\n') << time.c_str() << TEXT(" [INIT] ") << TEXT("Зарегистрирован запуск программы.\n");
			break;
		case INFO_MESSAGE:
			LogFile << time.c_str() << TEXT(" [INFO] ") << msg->c_str();
			break;
		case ERROR_MESSAGE:
			LogFile << time.c_str() << TEXT(" [ERROR] ") << msg->c_str();
			break;
		case CLOSE_MESSAGE:
			LogFile << time.c_str() << TEXT(" [CLOSE] ") << TEXT("Программа была закрыта.\n");
			LogFile.close();
			break;
		}
		return FUNC_SUCCESS;
	}
	// Обращаемся к обработчику ошибок
	catch (std::exception& e) {
		DebugInfo(CurFunc);
		LogFile.close();
		HandleError(WRITE_LOG_ERROR, NULL, e.what());
		return FUNC_ERROR;
	}
}

// Функция получения текущего системного времени
string GetSystemTime() {
	SYSTEMTIME cur_time;
	GetLocalTime(&cur_time);
	const uint8_t size = 7;
	const WORD* ptrData = (WORD*)&cur_time;
	string resultTime = TEXT("[");
	for (uint8_t i = 0; i < size; i++) {
		if (ptrData != &cur_time.wDayOfWeek) {
			if (*ptrData < 10) resultTime = resultTime + TEXT('0');
			resultTime = resultTime + toStr(*ptrData);
			if (ptrData == &cur_time.wDay) resultTime = resultTime + TEXT(' ');
			else if (ptrData != &cur_time.wSecond) resultTime = resultTime + TEXT('.');
		}
		ptrData++;
	}
	resultTime = resultTime + TEXT(']');
	return resultTime;
}
