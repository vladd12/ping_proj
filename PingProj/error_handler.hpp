#ifndef ERRHANDLE_H
#define ERRHANDLE_H

// Типы глобальных ошибок
#define INIT_LOGGER_ERROR			10
#define WRITE_LOG_ERROR				20
#define INIT_NETSUBSYSTEM_ERROR		30
#define BAD_ARGS_ERROR				40
#define INIT_SOCKET_ERROR			50
#define SEND_REQUEST_ERROR			60
#define GET_REPLY_ERROR				70
#define STATISTICS_ERROR			80
#define ERROR_HANDLE_FAILURE		100

// Определения некоторых дополнительных ошибок
#define WSAVERNOTFOUND			10011L
#define PARAMFEWPASSED			4010L

/*-------- Прототипы функций --------*/
//
void HandleError(int, int, const char* const);
string ConvertCharToWstring(const char* const);
BOOL SetConsoleText(WORD);
void UsageOutput();
void CloseProgram(BOOL, SOCKET*);

// Определения для функций вывода текста в консоль
#define RedColorText() SetConsoleText(FOREGROUND_RED)
#define WhiteColorText() SetConsoleText(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)

/*-- Вывод информации в режиме отладки --*/
#ifdef _DEBUG
	#define CurFunc			TEXT("Ошибка в функции: ") << ConvertCharToWstring(__func__).c_str()
	#define DebugInfo(q)	RedColorText(); cout << q << endl; WhiteColorText();
#else
	#define CurFunc
	#define DebugInfo(q)
#endif

#endif
