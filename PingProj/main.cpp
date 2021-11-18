#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

// Выясняем кодировку: использовать Юникод или ASCII кодировку
#ifdef UNICODE
	#define cout std::wcout
#else
	#define cout std::cout
#endif

void InitSock(WSADATA&);

// Главная функция программы
int main(int argc, TCHAR* argv[]) {
	WSADATA wsaData;
	
	InitSock(wsaData);
	WSACleanup();
	system("pause");
	return 0;
}

// 
void InitSock(WSADATA& wsaData) {
	int errorStateCode;
	const int sockVersion = 2;
	errorStateCode = WSAStartup(MAKEWORD(sockVersion, sockVersion), &wsaData);
	// Проверка на ошибки
	if (errorStateCode != 0) {
		cout << TEXT("Ошибка инициализации WinSock #");
		cout << errorStateCode << TEXT(":\n");
		switch (errorStateCode) {
		case WSASYSNOTREADY:
			cout << TEXT("Базовая сетевая подсистема не готова к сетевому взаимодействию.\n");
			break;
		case WSAVERNOTSUPPORTED:
			cout << TEXT("Запрошенная версия сокетов не поддерживается данной реализацией сокетов Windows.\n");
			break;
		case WSAEINPROGRESS:
			cout << TEXT("Выполняется операция блокировки сокетов.\n");
			break;
		case WSAEPROCLIM:
			cout << TEXT("Достигнуто ограничение на количество задач, поддерживаемых реализацией сокетов Windows.\n");
			break;
		case WSAEFAULT:
			cout << TEXT("Функции передан недопустимый указатель.\n");
			break;
		default:
			cout << TEXT("Сетевая подсистема вернула недопустимое значение.\n");
			break;
		}
		exit(1);
	}
	else if (LOBYTE(wsaData.wVersion) != sockVersion ||
	  HIBYTE(wsaData.wVersion) != sockVersion) {
		cout << TEXT("Не найдена указанная версия Winsock.dll.\n");
		WSACleanup();
		exit(2);
	}
	else cout << TEXT("Всё в порядке.\n");
}
