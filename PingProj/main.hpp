#ifndef MAIN_H
#define MAIN_H

#include <fstream>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <stdio.h>
#include <vector>

using std::vector;
using std::endl;

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
#endif

#define FUNC_SUCCESS	0	// Функция завершена удачно
#define FUNC_ERROR		1	// Функция завершилась с ошибкой

// Размер информации, хранимой в эхо-запросе
const UINT DATA_SIZE = 32;

// Структура, описывающая заголовок ICMP пакета
typedef struct tagICMPHDR {
	uint8_t Type;
	uint8_t Code;
	uint16_t Checksum;
	uint16_t ID;
	uint16_t Seq;
} ICMPHDR, *PICMPHDR;

// 
typedef struct tagECHOREQUEST {
	ICMPHDR icmpHdr; // Header
	int dwTime; // Time
	char cData[DATA_SIZE]; // Fill data
} ECHOREQUEST, *PECHOREQUEST;



int InitNetworkSubsystem(WSADATA&);
int CheckParams(int, TCHAR*[], IN_ADDR&);
int CorrectIP_DNS(TCHAR*, IN_ADDR&);
int InitSocks(SOCKADDR_IN&, SOCKADDR_IN&, SOCKADDR_IN&, SOCKET&, IN_ADDR&, WORD&, DWORD&);


#endif
