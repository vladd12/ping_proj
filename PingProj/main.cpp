#include "main.hpp"
#include "logger.hpp"

#pragma comment(lib, "Ws2_32.lib")

// Главная функция программы
int main(int argc, TCHAR* argv[]) {
	setlocale(LC_ALL, dLocale);
	//cout << sizeof(ECHOREPLY) << endl;
	// TCHAR* argarr[] = {L"prog", L"8.8.8.8"};
	WSADATA wsaData;
	IN_ADDR IP_Address;
	SOCKADDR_IN remote;
	SOCKADDR_IN bind;
	SOCKADDR_IN local;
	SOCKET listen;
	listen = NULL;
	WORD port;
	port = 1234;
	DWORD timeout;
	timeout = 5000;
	UINT steps, seq;
	steps = 4;
	seq = 1;
	clock_t start, end;

	// Главная хрень
	switch (InitLogger()) {
	case FUNC_SUCCESS:
		switch (InitNetworkSubsystem(wsaData)) {
		case FUNC_SUCCESS:
			switch (CheckParams(argc, argv, IP_Address)) {
			case FUNC_SUCCESS:
				switch (InitSocks(remote, bind, local,
					listen, IP_Address, port, timeout)) {
				case FUNC_SUCCESS:
					while (seq <= steps) {
						switch (SendRequest(remote, listen, seq, start)) {
						case FUNC_SUCCESS:
							switch (GetReply(remote, local, listen, start, end)) {
							case FUNC_SUCCESS:
								cout << 7 << endl;
								break;
							case FUNC_ERROR:
								/// TODO: Доделать перехватчик ошибок
								cout << 6 << endl;
								break;
							}
							break;
						case FUNC_ERROR:
							/// TODO: Доделать перехватчик ошибок
							cout << 5 << endl;
							return 1;
							break;
						}
						seq++;
					}
					break;
				case FUNC_ERROR:
					/// TODO: Доделать перехватчик ошибок
					cout << 4 << endl;
					break;
				}
				break;
			case FUNC_ERROR:
				/// TODO: Доделать перехватчик ошибок
				cout << 3 << endl;
				break;
			}
			break;
		case FUNC_ERROR:
			/// TODO: Доделать перехватчик ошибок
			cout << 2 << endl;
			break;
		}
		break;
	case FUNC_ERROR:
		/// TODO: Доделать перехватчик ошибок
		cout << 1 << endl;
		break;
	}



	/// TODO: Вынести в final, потом убрать
	closesocket(listen);
	WSACleanup();
	system("pause");
	return 0;
}

// Запуск сетевой подсистемы Windows
int InitNetworkSubsystem(WSADATA& wsaData) {
	// Инициализация сетевой подсистемы
	int errorStateCode;
	const int sockVersion = 2;
	errorStateCode = WSAStartup(MAKEWORD(sockVersion, sockVersion), &wsaData);

	// Проверка на ошибки
	/// TODO: Перенести код в логгер
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
		return FUNC_ERROR;
	}
	/// TODO: Перенести код в логгер
	else if (LOBYTE(wsaData.wVersion) != sockVersion ||
		HIBYTE(wsaData.wVersion) != sockVersion) {
		cout << TEXT("Не найдена указанная версия Winsock.dll.\n");
		WSACleanup();
		return FUNC_ERROR;
	}
	else return FUNC_SUCCESS;
}

// Проверка переданных программе параметров
int CheckParams(int argc, TCHAR* argv[], IN_ADDR& IPtoNum) {
	// Проверка на кол-во параметров
	if (argc < 2) {
		/// TODO:
		// Код для логгера и перехватчика ошибок
		return FUNC_ERROR;
	}
	// Проверка на корректность введённого IP/DNS адреса
	else if (CorrectIP_DNS(argv[1], IPtoNum) == FUNC_ERROR) {
		/// TODO:
		// Код для логгера и перехватчика ошибок
		return FUNC_ERROR;
	}
	//
	else {
		/// TODO: Доделать, если будет несколько флагов
		/*
		// Если больше двух параметром, обрабатываем их
		if (argc > 2) {
			vector<string> params;
			int i;
			for (i = 2; i < argc; i++) {
				params.push_back(string(argv[i]));
				return FUNC_SUCCESS;
			}
		}
		else
		*/
			return FUNC_SUCCESS;
	}
}

// Проверка корректности написания IP, 
int CorrectIP_DNS(TCHAR* IP, IN_ADDR& IPtoNum) {
	int erStat;	
	erStat = inet_pton(AF_INET, IP, &IPtoNum);
	if (IPtoNum.S_un.S_addr == INADDR_NONE) {
		LPSOCKADDR sockaddr_ip;
		ADDRINFO* result = NULL;
		ADDRINFO hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_ICMP;
		int errorStateCode;
		errorStateCode = getaddrinfo(IP, NULL, &hints, &result);
		if (errorStateCode != 0) {
			/// TODO: Доделать перехватчик ошибок
			// Код для логгера и перехватчика ошибок
			return FUNC_ERROR;
		}
		else {
			LPIN_ADDR ipNum;
			sockaddr_ip = result->ai_addr;
			ipNum = &((LPSOCKADDR_IN)sockaddr_ip)->sin_addr;
			IPtoNum = *ipNum;
			return FUNC_SUCCESS;
		}
	}
	else return FUNC_SUCCESS;
}

// Инициализация сетевой подсистемы и сокета
int InitSocks(SOCKADDR_IN& remote, SOCKADDR_IN& bind, SOCKADDR_IN& local, SOCKET& listen,
	IN_ADDR& ip_num, WORD& port, DWORD& timeout) {
	// Создаём сокет и проверяем на наличие ошибок при создании
	int errorStateCode;
	listen = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	//listen = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (listen == INVALID_SOCKET || listen == NULL) {
		/// TODO: Доделать перехватчик ошибок
		// Код для логгера и перехватчика ошибок
		errorStateCode = WSAGetLastError();
		cout << TEXT("Ошибка создания сокета: # ") << errorStateCode << TEXT(":\n");

		/// TODO: Вынести в финальную функцию
		// Код для логгера и перехватчика ошибок
		closesocket(listen);
		WSACleanup();
		return FUNC_ERROR;
	}

	// Полученный сокет будет принимать данные
	errorStateCode = setsockopt(listen, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	if (errorStateCode != 0) {
		/// TODO: Вынести в финальную функцию
		// Код для логгера и перехватчика ошибок
		return FUNC_ERROR;
	}

	// Настройка адресов
	remote.sin_addr = ip_num;
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	bind.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind.sin_family = AF_INET;
	bind.sin_port = htons(port);
	local.sin_family = AF_INET;
	return FUNC_SUCCESS;
}

// Формируем эхо-запрос и посылаем его удалённому устройству
int SendRequest(SOCKADDR_IN& remote, SOCKET& listen, WORD SeqNum, clock_t& start) {
	try {
		// Формируем эхо-запрос
		ECHOREQUEST req;
		req.icmpHdr.Type = 8;
		req.icmpHdr.Code = 0;
		req.icmpHdr.ID = (uint16_t)GetCurrentProcessId();
		req.icmpHdr.Seq = SeqNum;
		memset(req.cData, char(0xFF), DATA_SIZE);
		req.dwTime = GetTickCount();
		req.icmpHdr.Checksum = CRC16((uint8_t*)&req, sizeof(req));
		
		// Отправляем данные
		int erStat;
		erStat = sendto(listen, (char*)&req, sizeof(req), 0, (sockaddr*)&remote, sizeof(remote));
		start = clock();
		/// TODO: Логгер должен записать, что отправлено сообщение на этот адресс
		if (erStat == SOCKET_ERROR) {
			erStat = WSAGetLastError();
			/// TODO: Обработчик ошибок должен принять и обработать erStat
			return FUNC_ERROR;
		}
		return FUNC_SUCCESS;
	}
	catch (std::exception& e) {
		/// TODO: Или обращаться к обработчику ошибок
		cout << e.what() << endl;
		return FUNC_ERROR;
	}
}

// Функция для расчёта контрольной суммы CRC16
uint16_t CRC16(const uint8_t* data_p, unsigned int length) {
	uint16_t x;
	uint16_t crc = 0xFFFF;
	while (length--) {
		x = crc >> 8 ^ *data_p++;
		x ^= x >> 4;
		crc = (crc << 8) ^ (x << 12) ^ (x << 5) ^ x;
	}
	return crc;
}

// Функция для получения эхо-ответа
int GetReply(SOCKADDR_IN& remote, SOCKADDR_IN& local, SOCKET& listen, clock_t& start, clock_t& end) {
	try {
		const int BUFFER_SIZE = 256;
		char buffer[BUFFER_SIZE];
		bool timeout;
		timeout = false;
		int local_size;
		local_size = sizeof(local);

		// Получаем эхо-ответ
		if (recvfrom(listen, buffer, BUFFER_SIZE, 0, (sockaddr*)&local, &local_size) == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				/// TODO: Запись логгером, что превышено время ожидания
				cout << TEXT("Узел: ");
				ShowIpAddress(remote.sin_addr);
				cout << TEXT(".\nПревышен интервал ожидания запроса.\n");
				timeout = true;
			}
			else {
				/// TODO: Добавить код обработчика ошибок и логгера
				return FUNC_ERROR;
			}
		}
		else end = clock();

		// Если время ожидания не истекло
		if (!timeout) {
			PECHOREPLY ptrReply;
			ptrReply = (ECHOREPLY*)buffer;
			if (ptrReply->EchoRequest.icmpHdr.ID == GetCurrentProcessId()) {
				clock_t time = end - start;
				cout << TEXT("Ответ от ");
				ShowIpAddress(remote.sin_addr);
				cout << TEXT(": время обмена данными = ") << time << TEXT(" мс.\n");
			}
			else cout << TEXT("Принят ложный пакет.\n");
		}
		return FUNC_SUCCESS;
	}
	catch (std::exception& e) {
		/// TODO: Или обращаться к обработчику ошибок
		cout << e.what() << endl;
		return FUNC_ERROR;
	}
}

// Функция для вывода IP-адреса конечного узла
void ShowIpAddress(IN_ADDR& ip_num) {
	uint8_t b1, b2, b3, b4;
	b1 = ip_num.S_un.S_un_b.s_b1;
	b2 = ip_num.S_un.S_un_b.s_b2;
	b3 = ip_num.S_un.S_un_b.s_b3;
	b4 = ip_num.S_un.S_un_b.s_b4;
	cout << b1 << TEXT('.') << b2 << TEXT('.') << b3 << TEXT('.') << b4;
}



#ifdef _DEBUG 
#define num1 1
#else
#define num2 2
#endif
