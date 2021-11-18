#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>

// Выясняем кодировку: использовать Юникод или ASCII кодировку
#ifdef UNICODE
	#define cout std::wcout
#else
	#define cout std::cout
#endif

#pragma comment(lib, "Ws2_32.lib")

// 
int main(int argc, TCHAR* argv[]) {

	system("pause");
	return 0;
}
