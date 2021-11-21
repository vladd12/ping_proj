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
		if (WriteMessage(INIT_MESSAGE, NULL, NULL) == FUNC_ERROR) {
			/// TODO: Или обращаться к обработчику ошибок
			return FUNC_ERROR;
		}
		else return FUNC_SUCCESS;
	}
	catch (std::exception& e) {
		/// TODO: Или обращаться к обработчику ошибок
		cout << e.what() << endl;
		return FUNC_ERROR;
	}
}

// Запись входящего сообщения с помощью логгера
int WriteMessage(int type, int errorCode, UINT dTime) {
	try {
		string time;
		time = GetSystemTime();
		switch (type) {
		case INIT_MESSAGE:
			LogFile << TEXT("\n[INIT] ") << time.c_str() << TEXT(" Зарегистрирован запуск программы.\n");
			break;
		case INFO_MESSAGE:
			break;
		case ERROR_MESSAGE:
			break;
		case CLOSE_MESSAGE:
			LogFile << TEXT("[CLOSE] ") << time.c_str() << TEXT(" Программа была закрыта.\n");
			LogFile.close();
			break;
		}
		return FUNC_SUCCESS;
	}
	catch (std::exception& e) {
		/// TODO: Или обращаться к обработчику ошибок
		cout << e.what() << endl;
		return FUNC_ERROR;
	}

}

// Функция получения текущего системного времени
string GetSystemTime() {
	SYSTEMTIME cur_time;
	GetLocalTime(&cur_time);
	string resultTime;
	resultTime = TEXT('[') + toStr(cur_time.wDay) + TEXT('.') + toStr(cur_time.wMonth) + TEXT('.') + toStr(cur_time.wYear)
		+ TEXT("   ") + toStr(cur_time.wHour) + TEXT(':') + toStr(cur_time.wMinute) + TEXT(':') + toStr(cur_time.wSecond) + TEXT(']');
	return resultTime;
}
