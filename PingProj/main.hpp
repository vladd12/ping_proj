#ifndef MAIN_H
#define MAIN_H

#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <tchar.h>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

using std::exception;
using std::vector;
using std::endl;

// Подключение статической библиотеки
#pragma comment(lib, "Ws2_32.lib")

// Выясняем кодировку: использовать Юникод или ASCII кодировку
#ifdef UNICODE
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
#else
	#define cout std::cout
	#define dLocale "Russian"
	#define string std::string
	#define rFile std::ifstream
	#define wFile std::ofstream
	#define toStr(quote) std::to_string(quote)
	#define inet_pton inet_pton
	#define getaddrinfo getaddrinfo
	#define ADDRINFO ADDRINFOA
	#define Convert(quote) string(quote)
#endif

#define FUNC_SUCCESS	0	// Функция завершена удачно
#define FUNC_ERROR		1	// Функция завершилась с ошибкой

// Размер информации, хранимой в эхо-запросе
const UINT DATA_SIZE = 32;

// Структура, описывающая заголовок ICMP-пакета (RFC 792). Размер: 8 байт
typedef struct tagICMPHDR {
	uint8_t Type;					// Тип (8 для эхо-запроса, 0 для эхо-ответа)
	uint8_t Code;					// Код (0 для эхо-запроса и эхо-ответа)
	uint16_t Checksum;				// Контрольная сумма
	uint16_t ID;					// Идентификатор процесса, который послал эхо-запрос
	uint16_t Seq;					// Идентификатор последовательности
} ICMPHDR, *PICMPHDR;				// Псевдонимы типа для структуры

// Структура, описывающая эхо-запрос. Размер: динамический, в нашем случае 44 байта
typedef struct tagECHOREQUEST {
	ICMPHDR icmpHdr;				// Заголовок ICMP для эхо-запроса
	uint32_t dwTime;				// Временной штамп (информация о времени)
	char cData[DATA_SIZE];			// Информация для заполнения эхо-запроса
} ECHOREQUEST, *PECHOREQUEST;		// Псевдонимы типа для структуры

// Структура, описывающая заголовок IPv4-пакета. Размер: 20 байт
typedef struct tagIPHDR {
	uint8_t VIHL;					// Версия и IHL (отвечает за размер полей)
	uint8_t TOS;					// DSCP (6 бит, RFC 2474) и ECN (2 бита, RFC 3168)
	uint16_t Length;				// Длина пакета
	uint16_t ID;					// Идентификатор пакета
	uint16_t FlagOff;				// Флаги (3 бита) и смещения фрагментов (13 битов)
	uint8_t TTL;					// Время жизни пакета
	uint8_t Protocol;				// Идентификатор протокола транспортного уровня
	uint16_t Checksum;				// Контрольная сумма пакета
	in_addr iaSrc;					// IP-адрес отправителя (4 байта)
	in_addr iaDst;					// IP-адрес получателя (4 байта)
} IPHDR, *PIPHDR;					// Псевдонимы типа для структуры

// Структура, описывающая эхо-ответ. Размер: динамический, в нашем случае 64 байт
typedef struct tagECHOREPLY {
	IPHDR ipHdr;					// Заголовок IP для эхо-ответа
	ECHOREQUEST EchoRequest;		// Эхо-запрос
} ECHOREPLY, *PECHOREPLY;			// Псевдонимы типа для структуры

// Структура, описывающая статистку отправленных, полученных и потерянных пакетов
typedef struct tagSTAT {
	UINT sended;
	UINT received;
	UINT lost;
} STAT, *PSTAT;

/*----- Прототипы функций -----*/
//
int InitNetworkSubsystem(WSADATA&);
int CheckParams(int, TCHAR*[], IN_ADDR&);
int CorrectIP_DNS(TCHAR*, IN_ADDR&);
string IPtoStr(IN_ADDR&);
int InitSocks(SOCKADDR_IN&, SOCKADDR_IN&, SOCKADDR_IN&, SOCKET&, IN_ADDR&, WORD&, DWORD&);
int SendRequest(SOCKADDR_IN&, SOCKET&, WORD, clock_t&, STAT&, string&);
uint16_t CRC16(uint16_t*, unsigned int);
int GetReply(SOCKADDR_IN&, SOCKADDR_IN&, SOCKET&, clock_t&, clock_t&, STAT&, vector<clock_t>&, string&);
int Statistics(UINT&, STAT&, vector<clock_t>&, string&);

#endif
