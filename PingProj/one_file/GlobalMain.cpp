/* ----- Декларация библиотек ----- */
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <tchar.h>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

// Стандартные контейнеры из std
using std::locale;
using std::exception;
using std::vector;
using std::endl;

// Подключение статической библиотеки
#pragma comment(lib, "Ws2_32.lib")

/* ----- Декларация макросов ----- */
//
#define cout std::wcout
#define dLocale ""
#define string std::wstring
#define rFile std::wifstream
#define wFile std::wofstream
#define toStr(quote) std::to_wstring(quote)
#define inet_pton InetPtonW
#define getaddrinfo GetAddrInfoW 
#define ADDRINFO ADDRINFOW
#define Convert(quote) ConvertCharToWstring(quote)

// Коды возврата из функций
#define FUNC_SUCCESS				0	// Функция завершена удачно
#define FUNC_ERROR					1	// Функция завершилась с ошибкой

// Типы сообщений, передаваемые логгеру
#define INIT_MESSAGE				0	// Сообщение о запуске программы
#define INFO_MESSAGE				1	// Сообщение об отправке, получении данных или подведение статистики
#define ERROR_MESSAGE				2	// Сообщение об ошибке во время выполнения программы
#define CLOSE_MESSAGE				3	// Сообщение о закрытии программы

// Типы глобальных ошибок
#define INIT_LOGGER_ERROR			10	// Ошибка открытия файла для логов
#define WRITE_LOG_ERROR				20	// Ошибка записи в файл для логов
#define INIT_NETSUBSYSTEM_ERROR		30	// Ошибка инициализации сетевой подсистемы Windows
#define BAD_ARGS_ERROR				40	// Ошибка при работе с переданными программе аргументами
#define INIT_SOCKET_ERROR			50	// Ошибка создания сокета и его соединения к удалённому узлу
#define SEND_REQUEST_ERROR			60	// Ошибка передачи эхо-запроса
#define GET_REPLY_ERROR				70	// Ошибка получения эхо-ответа
#define STATISTICS_ERROR			80	// Ошибка при получении статистики работы программы
#define ERROR_HANDLE_FAILURE		100	// Ошибка при перехвате других ошибок

// Определения некоторых дополнительных ошибок
#define WSAVERNOTFOUND			10011L	// Невозможно работать с версией сокетов 2.2
#define PARAMFEWPASSED			4010L	// Передано малое число параметров

/* ----- Декларация структур ----- */
//
const UINT DATA_SIZE = 32;	// Размер информации, хранимой в эхо-запросе

// Структура, описывающая заголовок ICMP-пакета (RFC 792). Размер: 8 байт
typedef struct tagICMPHDR {
	uint8_t Type;				// Тип (8 для эхо-запроса, 0 для эхо-ответа)
	uint8_t Code;				// Код (0 для эхо-запроса и эхо-ответа)
	uint16_t Checksum;			// Контрольная сумма
	uint16_t ID;				// Идентификатор процесса, который послал эхо-запрос
	uint16_t Seq;				// Идентификатор последовательности
} ICMPHDR, *PICMPHDR;			// Псевдонимы типа для структуры

// Структура, описывающая эхо-запрос. Размер: динамический, в нашем случае 44 байта
typedef struct tagECHOREQUEST {
	ICMPHDR icmpHdr;			// Заголовок ICMP для эхо-запроса
	uint32_t dwTime;			// Временной штамп (информация о времени)
	char cData[DATA_SIZE];		// Информация для заполнения эхо-запроса
} ECHOREQUEST, *PECHOREQUEST;	// Псевдонимы типа для структуры

// Структура, описывающая заголовок IPv4-пакета. Размер: 20 байт
typedef struct tagIPHDR {
	uint8_t VIHL;				// Версия и IHL (отвечает за размер полей)
	uint8_t TOS;				// DSCP (6 бит, RFC 2474) и ECN (2 бита, RFC 3168)
	uint16_t Length;			// Длина пакета
	uint16_t ID;				// Идентификатор пакета
	uint16_t FlagOff;			// Флаги (3 бита) и смещения фрагментов (13 битов)
	uint8_t TTL;				// Время жизни пакета
	uint8_t Protocol;			// Идентификатор протокола транспортного уровня
	uint16_t Checksum;			// Контрольная сумма пакета
	in_addr iaSrc;				// IP-адрес отправителя (4 байта)
	in_addr iaDst;				// IP-адрес получателя (4 байта)
} IPHDR, *PIPHDR;				// Псевдонимы типа для структуры

// Структура, описывающая эхо-ответ. Размер: динамический, в нашем случае 64 байт
typedef struct tagECHOREPLY {
	IPHDR ipHdr;				// Заголовок IP для эхо-ответа
	ECHOREQUEST EchoRequest;	// Эхо-запрос
} ECHOREPLY, *PECHOREPLY;		// Псевдонимы типа для структуры

// Структура, описывающая статистку отправленных, полученных и потерянных пакетов
typedef struct tagSTAT {
	UINT sended;				// Кол-во отправленных запросов
	UINT received;				// Кол-во полученных ответов
	UINT lost;					// Кол-во потерянных пакетов
} STAT, *PSTAT;					// Псевдонимы типа для структуры

/* ----- Декларация глобальных переменных ----- */
wFile LogFile;					// Файл лога
int MajorErrorCode;				// Глобальный код ошибки
int MinorErrorCode;				// Уточняющий код ошибки
string errString;				// Строка для записей в лог сообщений типа ERROR
string infoMessage;				// Строка для записей в лог сообщений типа INFO
IN_ADDR IP_Address;				// Структура, хранящая IP-адрес удалённого узла
string IP_Str;					// Строковое представление IP-адреса удалённого узла
SOCKADDR_IN remote;				// Структура, описывающая сокет удалённого устройства
SOCKADDR_IN bind_port;			// Структура, описывающая сокет локального устройства
SOCKADDR_IN local;				// Структура, используемая для передачи и получения информация
SOCKET listenSock;				// Сокет для приёма и передачи данных
DWORD timeout;					// Время ожидания ответа от удалённого узла
WORD port;						// Номер порта, на котором будет открыт сокет для отправки и принятия данных
UINT steps, seq;				// Переменные для цикла, хранят кол-во запросов и номер текущего запроса
clock_t start, end;				// Переменные, хранящие информацию о времени отправки (start) запроса и получения ответа (end)
STAT statistics;				// Структура для подведения статистики
vector<clock_t> time_vector;	// Массив, хранящий время отправки-получения каждого запроса

/* ----- Декларация функций ----- */
//
int InitLogger();
int WriteMessage(int, string*);
void HandleError(int, int, string*);
int InitNetworkSubsystem();
int CheckParams(int, TCHAR*[], IN_ADDR&);
int CorrectIP_DNS(TCHAR*, IN_ADDR&);
string IPtoStr(IN_ADDR&);
int InitSocks(SOCKADDR_IN&, SOCKADDR_IN&, SOCKADDR_IN&, SOCKET&, IN_ADDR&, WORD&, DWORD&);
int SendRequest(SOCKADDR_IN&, SOCKET&, WORD, clock_t&, STAT&, string&);
uint16_t CRC16(uint16_t*, unsigned int);
int GetReply(SOCKADDR_IN&, SOCKADDR_IN&, SOCKET&, clock_t&, clock_t&, STAT&, vector<clock_t>&, string&);
int Statistics(UINT&, STAT&, vector<clock_t>&, string&);
string GetSystemTime();
string ConvertCharToWstring(const char* const);
BOOL SetConsoleText(WORD);
void UsageOutput();
void CloseProgram(BOOL, SOCKET*);

/* ----- Дополнительные макросы ----- */
//
// Макросы изменения цвета текста в консоли
#define RedColorText() SetConsoleText(FOREGROUND_RED)
#define WhiteColorText() SetConsoleText(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)

// Макросы вывода информации при сборке проекта в режиме отладки
#ifdef _DEBUG
	#define CurFunc			TEXT("Ошибка в функции: ") << ConvertCharToWstring(__func__).c_str()
	#define DebugInfo(q)	RedColorText(); cout << q << endl; WhiteColorText();
#else
	#define CurFunc
	#define DebugInfo(q)
#endif

/* -- Определения функций -- */
//
// Главная функция программы
int _tmain(int argc, TCHAR* argv[]) {
	/* -- Инициализация глобальных переменных -- */
	setlocale(LC_ALL, dLocale);
	MajorErrorCode = EXIT_SUCCESS;
	MinorErrorCode = NULL;
	listenSock = NULL;
	port = 1234;
	timeout = 5000;
	steps = 4;
	seq = 0;
	statistics = { 0 };

	// Главная конструкция, реализующая конечный автомат
	switch (InitLogger()) {
	case FUNC_SUCCESS:
		switch (WriteMessage(INIT_MESSAGE, NULL)) {
		case FUNC_SUCCESS:
			switch (InitNetworkSubsystem()) {
			case FUNC_SUCCESS:
				switch (CheckParams(argc, argv, IP_Address)) {
				case FUNC_SUCCESS:
					IP_Str = IPtoStr(IP_Address);
					switch (InitSocks(remote, bind_port, local,
						listenSock, IP_Address, port, timeout)) {
					case FUNC_SUCCESS:
						// Цикличная отправка данных
						while (seq < steps) {
							switch (SendRequest(remote, listenSock, seq, start, statistics, IP_Str)) {
							case FUNC_SUCCESS:
								switch (WriteMessage(INFO_MESSAGE, &infoMessage)) {
								case FUNC_SUCCESS:
									switch (GetReply(remote, local, listenSock, start, end, statistics, time_vector, IP_Str)) {
									case FUNC_SUCCESS:
										switch (WriteMessage(INFO_MESSAGE, &infoMessage)) {
										case FUNC_SUCCESS:
											break;
										case FUNC_ERROR:
											HandleError(MajorErrorCode, MinorErrorCode, &errString);
											CloseProgram(TRUE, &listenSock);
											break;
										}
										break;
									case FUNC_ERROR:
										HandleError(MajorErrorCode, MinorErrorCode, &errString);
										switch (WriteMessage(ERROR_MESSAGE, &errString)) {
										case FUNC_SUCCESS:
											CloseProgram(TRUE, &listenSock);
											break;
										case FUNC_ERROR:
											HandleError(MajorErrorCode, MinorErrorCode, &errString);
											CloseProgram(TRUE, &listenSock);
											break;
										}
										break;
									}
									break;
								case FUNC_ERROR:
									HandleError(MajorErrorCode, MinorErrorCode, &errString);
									CloseProgram(TRUE, &listenSock);
									break;
								}
								break;
							case FUNC_ERROR:
								HandleError(MajorErrorCode, MinorErrorCode, &errString);
								switch (WriteMessage(ERROR_MESSAGE, &errString)) {
								case FUNC_SUCCESS:
									CloseProgram(TRUE, &listenSock);
									break;
								case FUNC_ERROR:
									HandleError(MajorErrorCode, MinorErrorCode, &errString);
									CloseProgram(TRUE, &listenSock);
									break;
								}
								break;
							}
							seq++;
						}
						// Подведение статистики
						switch (Statistics(steps, statistics, time_vector, IP_Str)) {
						case FUNC_SUCCESS:
							switch (WriteMessage(INFO_MESSAGE, &infoMessage)) {
							case FUNC_SUCCESS:
								// Штатное закрытие программы
								CloseProgram(TRUE, &listenSock);
								break;
							case FUNC_ERROR:
								HandleError(MajorErrorCode, MinorErrorCode, &errString);
								CloseProgram(TRUE, &listenSock);
								break;
							}
							break;
						case FUNC_ERROR:
							HandleError(MajorErrorCode, MinorErrorCode, &errString);
							switch (WriteMessage(ERROR_MESSAGE, &errString)) {
							case FUNC_SUCCESS:
								CloseProgram(TRUE, &listenSock);
								break;
							case FUNC_ERROR:
								HandleError(MajorErrorCode, MinorErrorCode, &errString);
								CloseProgram(TRUE, &listenSock);
								break;
							}
							break;
						}
						break;
					case FUNC_ERROR:
						HandleError(MajorErrorCode, MinorErrorCode, &errString);
						switch (WriteMessage(ERROR_MESSAGE, &errString)) {
						case FUNC_SUCCESS:
							CloseProgram(TRUE, &listenSock);
							break;
						case FUNC_ERROR:
							HandleError(MajorErrorCode, MinorErrorCode, &errString);
							CloseProgram(TRUE, &listenSock);
							break;
						}
						break;
					}
					break;
				case FUNC_ERROR:
					HandleError(MajorErrorCode, MinorErrorCode, &errString);
					switch (WriteMessage(ERROR_MESSAGE, &errString)) {
					case FUNC_SUCCESS:
						CloseProgram(TRUE, NULL);
						break;
					case FUNC_ERROR:
						HandleError(MajorErrorCode, MinorErrorCode, &errString);
						CloseProgram(TRUE, NULL);
						break;
					}
					break;
				}
				break;
			case FUNC_ERROR:
				HandleError(MajorErrorCode, MinorErrorCode, &errString);
				switch (WriteMessage(ERROR_MESSAGE, &errString)) {
				case FUNC_SUCCESS:
					CloseProgram(TRUE, NULL);
					break;
				case FUNC_ERROR:
					HandleError(MajorErrorCode, MinorErrorCode, &errString);
					CloseProgram(TRUE, NULL);
					break;
				}
				break;
			}
			break;
		case FUNC_ERROR:
			HandleError(MajorErrorCode, MinorErrorCode, &errString);
			CloseProgram(FALSE, NULL);
			break;
		}
		break;
	case FUNC_ERROR:
		HandleError(MajorErrorCode, MinorErrorCode, &errString);
		CloseProgram(FALSE, NULL);
		break;
	}

	return 0;
}

// Функция для инициализации логгера
int InitLogger() {
	try {
		locale defaultLocale(dLocale);
		LogFile.imbue(defaultLocale);
		LogFile.open("log.txt", std::ios::app);
		return FUNC_SUCCESS;
	}
	// Обращаемся к обработчику ошибок
	catch (exception& e) {
		DebugInfo(CurFunc);
		LogFile.close();
		MajorErrorCode = INIT_MESSAGE;
		errString = Convert(e.what());
		return FUNC_ERROR;
	}
}

// Функция для записи сообщений в лог-файл
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
	catch (exception& e) {
		DebugInfo(CurFunc);
		LogFile.close();
		MajorErrorCode = WRITE_LOG_ERROR;
		errString = Convert(e.what());
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

// Запуск сетевой подсистемы Windows
int InitNetworkSubsystem() {
	// Инициализация сетевой подсистемы
	const byte sockVersion = 2;
	WSADATA wsaData;
	MinorErrorCode = WSAStartup(MAKEWORD(sockVersion, sockVersion), &wsaData);

	// Проверка на ошибки и перехват их возникновения
	if (MinorErrorCode != 0) {
		DebugInfo(CurFunc);
		MajorErrorCode = INIT_NETSUBSYSTEM_ERROR;
		return FUNC_ERROR;
	}
	else if (LOBYTE(wsaData.wVersion) != sockVersion ||
		HIBYTE(wsaData.wVersion) != sockVersion) {
		DebugInfo(CurFunc);
		MajorErrorCode = INIT_NETSUBSYSTEM_ERROR;
		MinorErrorCode = WSAVERNOTFOUND;
		return FUNC_ERROR;
	}
	else return FUNC_SUCCESS;
}

// Проверка переданных программе параметров
int CheckParams(int argc, TCHAR* argv[], IN_ADDR& IPtoNum) {
	// Проверка на кол-во параметров
	if (argc < 2) {
		DebugInfo(CurFunc);
		MajorErrorCode = BAD_ARGS_ERROR;
		MinorErrorCode = PARAMFEWPASSED;
		return FUNC_ERROR;
	}
	// Проверка на корректность введённого IP/DNS адреса
	else if (CorrectIP_DNS(argv[1], IPtoNum) == FUNC_ERROR) {
		// Minor error code устанавливается в CorrectIP_DNS
		MajorErrorCode = BAD_ARGS_ERROR;
		return FUNC_ERROR;
	}
	// Пройдены все проверки
	else return FUNC_SUCCESS;
}

// Проверка корректности написания IP, 
int CorrectIP_DNS(TCHAR* IP, IN_ADDR& IPtoNum) {
	MinorErrorCode = inet_pton(AF_INET, IP, &IPtoNum);
	if (MinorErrorCode == 0) {
		ADDRINFO* result = NULL;
		ADDRINFO hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_RAW;
		hints.ai_protocol = IPPROTO_ICMP;
		MinorErrorCode = getaddrinfo(IP, NULL, &hints, &result);
		if (MinorErrorCode != 0) return FUNC_ERROR;
		else {
			LPIN_ADDR ipNum;
			LPSOCKADDR sockaddr_ip;
			sockaddr_ip = result->ai_addr;
			ipNum = &((LPSOCKADDR_IN)sockaddr_ip)->sin_addr;
			IPtoNum = *ipNum;
			return FUNC_SUCCESS;
		}
	}
	else return FUNC_SUCCESS;
}

// Функция для перевода IP-адреса из структуры IN_ADDR в строку
string IPtoStr(IN_ADDR& ip_num) {
	string result;
	uint8_t ip1, ip2, ip3, ip4;
	TCHAR delim = TEXT('.');
	ip1 = ip_num.S_un.S_un_b.s_b1;
	ip2 = ip_num.S_un.S_un_b.s_b2;
	ip3 = ip_num.S_un.S_un_b.s_b3;
	ip4 = ip_num.S_un.S_un_b.s_b4;
	result = toStr(ip1) + delim + toStr(ip2) + delim + toStr(ip3) + delim + toStr(ip4);
	return result;
}

// Инициализация сетевой подсистемы и сокета
int InitSocks(SOCKADDR_IN& remote, SOCKADDR_IN& bind, SOCKADDR_IN& local,
	SOCKET& listen, IN_ADDR& ip_num, WORD& port, DWORD& timeout) {
	// Настройка адресов
	remote.sin_addr = ip_num;
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	bind.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind.sin_family = AF_INET;
	bind.sin_port = htons(port);
	local = { 0 };
	local.sin_family = AF_INET;

	// Создаём сокет и проверяем на наличие ошибок при создании
	listen = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	// Обработка ошибок, если они есть
	if (listen == INVALID_SOCKET || listen == NULL) {
		DebugInfo(CurFunc);
		MajorErrorCode = INIT_SOCKET_ERROR;
		MinorErrorCode = WSAGetLastError();
		return FUNC_ERROR;
	}

	// Полученный сокет будет принимать данные
	MinorErrorCode = setsockopt(listen, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	// Обработка ошибок, если они есть
	if (MinorErrorCode != 0) {
		DebugInfo(CurFunc);
		MajorErrorCode = INIT_SOCKET_ERROR;
		return FUNC_ERROR;
	}
	return FUNC_SUCCESS;
}

// Формируем эхо-запрос и посылаем его удалённому устройству
int SendRequest(SOCKADDR_IN& remote, SOCKET& listen, WORD SeqNum,
	clock_t& start, STAT& net_stat, string& ip_str) {
	try {
		// Вывод первой строки об обмене данными
		if (SeqNum == 0) cout << TEXT("Обмен пакетами с узлом ") << ip_str.c_str() << TEXT(":\n");

		// Формируем эхо-запрос
		ECHOREQUEST req;
		req = { 0 };
		req.icmpHdr.Type = 8;
		req.icmpHdr.Code = 0;
		req.icmpHdr.ID = (uint16_t)GetCurrentProcessId();
		req.icmpHdr.Seq = SeqNum;
		req.dwTime = GetTickCount();
		memset(req.cData, char('Z'), DATA_SIZE);
		req.icmpHdr.Checksum = CRC16((uint16_t*)&req, sizeof(req));

		// Отправляем эхо-запрос на удалённый узел
		MinorErrorCode = sendto(listen, (char*)&req, sizeof(req), 0,
			(sockaddr*)&remote, sizeof(remote));
		start = clock();

		// Проверка на наличие ошибок при отправлении запроса
		if (MinorErrorCode == SOCKET_ERROR) {
			DebugInfo(CurFunc);
			MajorErrorCode = SEND_REQUEST_ERROR;
			MinorErrorCode = WSAGetLastError();
			return FUNC_ERROR;
		}
		// Пишем в лог, что отправили эхо-запрос
		else {
			infoMessage = TEXT("На ") + ip_str + TEXT(" отправлено ")
				+ toStr(DATA_SIZE) + TEXT(" байт.\n");
			net_stat.sended++;
			return FUNC_SUCCESS;
		}
	}
	// Отлавливаем все возможные ошибки в ходе отправки запроса
	catch (exception& e) {
		DebugInfo(CurFunc);
		MajorErrorCode = SEND_REQUEST_ERROR;
		errString = Convert(e.what());
		return FUNC_ERROR;
	}
}

// Функция для расчёта контрольной суммы CRC16
uint16_t CRC16(uint16_t* addr, unsigned int length) {
	const uint32_t offset = 0x00000010;
	const uint32_t nand = 0x0000ffff;
	register uint32_t sum = 0;
	while (length > 1) {
		sum += *addr++;
		length -= sizeof(uint16_t);
	}
	if (length > 0) sum += *(uint8_t*)addr;
	while (sum >> offset) {
		sum = (sum & nand) + (sum >> offset);
	}
	return (uint16_t)(~sum);
}

// Функция для получения эхо-ответа
int GetReply(SOCKADDR_IN& remote, SOCKADDR_IN& local, SOCKET& listen,
	clock_t& start, clock_t& end, STAT& net_stat, vector<clock_t>& time_vec, string& ip_str) {
	try {
		const int BUFFER_SIZE = 256;
		char buffer[BUFFER_SIZE];
		bool timeout;
		timeout = false;
		int local_size;
		local_size = sizeof(local);

		// Получаем эхо-ответ
		if (recvfrom(listen, buffer, BUFFER_SIZE, 0, (sockaddr*)&local, &local_size) == SOCKET_ERROR) {
			MinorErrorCode = WSAGetLastError();
			if (MinorErrorCode == WSAETIMEDOUT) timeout = true;
			else {
				DebugInfo(CurFunc);
				HandleError(GET_REPLY_ERROR, MinorErrorCode, NULL);
				return FUNC_ERROR;
			}
		}
		else end = clock();

		// Если время ожидания не истекло
		if (!timeout) {
			PECHOREPLY ptrReply;
			ptrReply = (ECHOREPLY*)buffer;
			// Если ID совпадает с ID процесса
			if (ptrReply->EchoRequest.icmpHdr.ID == GetCurrentProcessId()) {
				// Выводим данные о полученном пакете в лог и консоль
				clock_t time = end - start;
				net_stat.received++;
				time_vec.push_back(time);
				uint8_t ttl = ptrReply->ipHdr.TTL;
				infoMessage = TEXT("Получен ответ от ") + ip_str
					+ TEXT(": число байт=") + toStr(DATA_SIZE) + TEXT(" время=")
					+ toStr(time) + TEXT("мс TTL=") + toStr(ttl) + TEXT(".\n");
				cout << infoMessage.c_str();
			}
			// Иначе выводим данные о ложном пакете в лог и консоль
			else {
				net_stat.lost++;
				infoMessage = TEXT("Принят ложный пакет.\n");
				cout << infoMessage.c_str();
			}
		}
		// Если время ожидания истекло
		else {
			// Выводим в консоль и лог информацию о таймауте
			infoMessage = TEXT("Узел: ") + ip_str + TEXT(".\nПревышен интервал ожидания запроса.\n");
			cout << infoMessage.c_str();
		}
		Sleep(1000);
		return FUNC_SUCCESS;
	}
	// Была получена ошибка во время выполнения
	catch (exception& e) {
		DebugInfo(CurFunc);
		MajorErrorCode = GET_REPLY_ERROR;
		errString = Convert(e.what());
		return FUNC_ERROR;
	}
}

// Функция для формирования статистики работы программы
int Statistics(UINT& steps, STAT& net_stat, vector<clock_t>& time_vec, string& ip_str) {
	try {
		// Формируем строку о переданных, полученных и потерянных пакетах
		UINT percent = 100 / steps;
		UINT percLost = percent * net_stat.lost;
		cout << TEXT("\nСтатистика ping для ") << ip_str.c_str() << TEXT(":\n    ");
		infoMessage = TEXT("Пакетов: отправлено=") + toStr(net_stat.sended)
			+ TEXT(", получено=") + toStr(net_stat.received)
			+ TEXT(", потеряно=") + toStr(net_stat.lost) + TEXT("    (")
			+ toStr(percLost) + TEXT("% потерь). ");

		// Выводим сформированную строку в консоль
		cout << infoMessage.c_str() << endl;

		// Находим минимальное, максимальное и среднее время
		// отправки-получения пакета
		size_t vecSize = time_vec.size();
		if (vecSize > 0) {
			clock_t min, max, avg;
			max = -1;
			min = 1000;
			avg = 0;
			for (size_t i = 0; i < vecSize; i++) {
				clock_t elem = time_vec[i];
				if (elem < min) min = elem;
				if (elem > max) max = elem;
				avg += elem;
			}
			avg = avg / vecSize;

			// Выводим информацию в консоль
			cout << TEXT("Приблизительное время приёма-передачи:\n    ");
			string info2 = TEXT("мин=") + toStr(min) + TEXT("мс, макс=")
				+ toStr(max) + TEXT("мс, среднее=") + toStr(avg) + TEXT("мс\n");
			cout << info2.c_str();
			info2 = TEXT("Статистика приёма-передачи пакетов: ") + info2;
			infoMessage += info2;
		}
		else cout << endl;
		return FUNC_SUCCESS;
	}
	// Была получена ошибка во время выполнения
	catch (exception& e) {
		DebugInfo(CurFunc);
		MajorErrorCode = STATISTICS_ERROR;
		errString = Convert(e.what());
		return FUNC_ERROR;
	}
}

// Функция, перехватывающая ошибки
void HandleError(int majorErrCode, int minorErrCode, string* errString) {
	try {
		// Выбираем, что делать
		switch (majorErrCode) {
		case INIT_NETSUBSYSTEM_ERROR:
			*errString = TEXT("Init Network Subsystem Failure #") + toStr(minorErrCode) + TEXT(": ");
			switch (minorErrCode) {
			case WSASYSNOTREADY:
				*errString += TEXT("Базовая сетевая подсистема не готова к сетевому взаимодействию.\n");
				break;
			case WSAVERNOTSUPPORTED:
				*errString += TEXT("Запрошенная версия сокетов не поддерживается данной реализацией сокетов Windows.\n");
				break;
			case WSAVERNOTFOUND:
				*errString += TEXT("Не найдена указанная версия Winsock.dll.\n");
				break;
			case WSAEINPROGRESS:
				*errString += TEXT("Выполняется операция блокировки сокетов.\n");
				break;
			case WSAEPROCLIM:
				*errString += TEXT("Достигнуто ограничение на количество задач, поддерживаемых реализацией сокетов Windows.\n");
				break;
			case WSAEFAULT:
				*errString += TEXT("Функции передан недопустимый указатель.\n");
				break;
			default:
				*errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
				break;
			}
			break;
		case BAD_ARGS_ERROR:
			*errString = TEXT("Bad Arguments Passed #") + toStr(minorErrCode) + TEXT(": ");
			switch (minorErrCode) {
			case PARAMFEWPASSED:
				*errString += TEXT("Передано слишком маленькое число параметров.\n");
				UsageOutput();
				break;
			case WSANO_RECOVERY:
				*errString += TEXT("Произошла неисправимая ошибка в разрешении имён.\n");
				break;
			case WSA_NOT_ENOUGH_MEMORY:
				*errString += TEXT("Произошёл сбой выделения памяти.\n");
				break;
			case WSAHOST_NOT_FOUND:
				*errString += + TEXT("Удалённый узел с указанным именем не найден.\n");
				break;
			case WSATYPE_NOT_FOUND:
				*errString += TEXT("Указанная служба не поддерживается указанным типом сокетов.\n");
				break;
			case WSAESOCKTNOSUPPORT:
				*errString += TEXT("Указанный сокет не поддерживается функцией getaddrinfo.\n");
				break;
			default:
				*errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
				break;
			}
			break;
		case INIT_SOCKET_ERROR:
			*errString = TEXT("Init Socket Failure #") + toStr(minorErrCode) + TEXT(": ");
			switch (minorErrCode) {
			case WSAENETDOWN:
				*errString += TEXT("Произошёл сбой сетевой подсистемы или отсутствует подключение к сети Интернет.\n");
				break;
			case WSAEINPROGRESS:
				*errString += TEXT("Выполняется блокирующий вызов сокетов, или поставщик услуг Интернет обрабатывает функцию обратного вызова.\n");
				break;
			case WSAEMFILE:
				*errString += TEXT("Не осталось доступных дескрипторов сокетов.\n");
				break;
			case WSAEINVALIDPROVIDER:
				*errString += TEXT("Поставщик услуг Интернет вернул сокет, отличный от указанного.\n");
				break;
			case WSAEINVALIDPROCTABLE:
				*errString += TEXT("Поставщик услуг Интернет вернул неверную или неполную таблицу процедур.\n");
				break;
			case WSAENOBUFS:
				*errString += TEXT("Буферное пространство недоступно. Сокет не может быть создан.\n");
				break;
			case WSAEPROVIDERFAILEDINIT:
				*errString += TEXT("Поставщику услуг Интернет не удалось выполнить инициализацию.\n");
				break;
			case WSAENOTSOCK:
				*errString += TEXT("Полученный дескриптор не является сокетом.\n");
				break;
			default:
				*errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
				break;
			}
			break;
		case SEND_REQUEST_ERROR:
			if (minorErrCode != NULL) {
				*errString = TEXT("Send Echo Request Failure #") + toStr(minorErrCode) + TEXT(": ");
				switch (minorErrCode) {
				case WSAENETDOWN:
					*errString += TEXT("Произошёл сбой сетевой подсистемы или отсутствует подключение к сети Интернет.\n");
					break;
				case WSAEINPROGRESS:
					*errString += TEXT("Выполняется блокирующий вызов сокетов, или поставщик услуг Интернет обрабатывает функцию обратного вызова.\n");
					break;
				case WSAEACCES:
					*errString += TEXT("Запрошенный адрес является широковещательным.\n");
					break;
				case WSAENETRESET:
					*errString += TEXT("Соединение было разорвано во время выполнения операции.\n");
					break;
				case WSAENOBUFS:
					*errString += TEXT("Буферное пространство недоступно.\n");
					break;
				case WSAEHOSTUNREACH:
					*errString += TEXT("Невозможно связаться с удалённым хостом.\n");
					break;
				case WSAECONNABORTED:
					*errString += TEXT("Виртуальный канал был прерван из-за тайм-аута или другого сбоя.\n");
					break;
				case WSAEADDRNOTAVAIL:
					*errString += TEXT("Удалённый адрес не является допустимым адресом.\n");
					break;
				case WSAENETUNREACH:
					*errString += TEXT("Невозможно подключиться к сети Интернет.\n");
					break;
				case WSAETIMEDOUT:
					*errString += TEXT("Соединение было прервано из-за сбоя сети или из-за того, что система на другом конце вышла из строя без уведомления.\n");
					break;
				default:
					*errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
					break;
				}
			}
			break;
		case GET_REPLY_ERROR:
			if (minorErrCode != NULL) {
				*errString = TEXT("Get Echo Reply Failure #") + toStr(minorErrCode) + TEXT(": ");
				switch (minorErrCode) {
				case WSAENETDOWN:
					*errString += TEXT("Произошёл сбой сетевой подсистемы или отсутствует подключение к сети Интернет.\n");
					break;
				case WSAEINPROGRESS:
					*errString += TEXT("Выполняется блокирующий вызов сокетов, или поставщик услуг Интернет обрабатывает функцию обратного вызова.\n");
					break;
				case WSAENETRESET:
					*errString += TEXT("Сетевая подсистема была сброшена.\n");
					break;
				case WSAECONNRESET:
					*errString += TEXT("Соединение было сброшено удалённой стороной.\n");
					break;
				default:
					*errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
					break;
				}
			}
			break;
		default:
			break;
		}

		// Запись об ошибке в консоль
		BOOL state = RedColorText();
		cout << (*errString).c_str() << endl;
		state = WhiteColorText();
	}

	// Ошибки при работе с ошибками
	catch (exception& e) {
		DebugInfo(CurFunc);
		cout << ConvertCharToWstring(e.what()).c_str() << endl;
		majorErrCode = ERROR_HANDLE_FAILURE;
		CloseProgram(FALSE, NULL);
	}
}

// Конвертация char в wchar_t
string ConvertCharToWstring(const char* const str) {
	const size_t cSize = strlen(str) + 1;
	size_t outSize;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs_s(&outSize, wc, cSize, str, cSize - 1);
	return string(wc);
}

// Функция для установки цвета выводимого текста в консоль
BOOL SetConsoleText(WORD Attributes) {
	HANDLE handle = NULL;
	handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (handle == NULL) return FALSE;
	else {
		BOOL result;
		result = SetConsoleTextAttribute(handle, Attributes);
		return result;
	}
}

// Вывод информации об использовании программы
inline void UsageOutput() {
	cout << TEXT("Запуск программы: ping <IP-адрес>\nили: ping <доменное_имя>\n");
}

// Функция для завершения работы программы
void CloseProgram(BOOL isWSAStarted, SOCKET* sock) {
	if (isWSAStarted == FALSE) exit(MajorErrorCode);
	else {
		if (sock != NULL) closesocket(*sock);
		WSACleanup();
		WriteMessage(CLOSE_MESSAGE, NULL);
		system("pause");
		exit(MajorErrorCode);
	}
}
