#include "logger.hpp"
#include "error_handler.hpp"

// Главная функция программы
int _tmain(int argc, TCHAR* argv[]) {
	setlocale(LC_ALL, dLocale);
	// cout << sizeof(ECHOREQUEST) << endl;
	// TCHAR* argarr[] = { L"prog", L"8.8.8.8" };
	WSADATA wsaData;
	IN_ADDR IP_Address;
	string IP_Str;
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
	seq = 0;
	clock_t start, end;
	STAT statistics;
	statistics = { 0 };
	vector<clock_t> time_vector;

	// Главная конструкция, осуществляющая конечный автомат
	switch (InitLogger()) {
	case FUNC_SUCCESS:
		switch (InitNetworkSubsystem(wsaData)) {
		case FUNC_SUCCESS:
			//switch (CheckParams(2, argarr, IP_Address)) {
			switch (CheckParams(argc, argv, IP_Address)) {
			case FUNC_SUCCESS:
				IP_Str = IPtoStr(IP_Address);
				switch (InitSocks(remote, bind, local,
					listen, IP_Address, port, timeout)) {
				case FUNC_SUCCESS:
					while (seq < steps) {
						switch (SendRequest(remote, listen, seq, start, statistics, IP_Str)) {
						case FUNC_SUCCESS:
							switch (GetReply(remote, local, listen, start, end, statistics, time_vector, IP_Str)) {
							case FUNC_SUCCESS:
								break;
							case FUNC_ERROR:
								CloseProgram(TRUE, &listen);
								break;
							}
							break;
						case FUNC_ERROR:
							CloseProgram(TRUE, &listen);
							break;
						}
						seq++;
					}
					// Подведение статистики
					switch (Statistics(steps, statistics, time_vector, IP_Str)) {
					case FUNC_SUCCESS:
						break;
					case FUNC_ERROR:
						CloseProgram(TRUE, &listen);
						break;
					}
					break;
				case FUNC_ERROR:
					CloseProgram(TRUE, &listen);
					break;
				}
				break;
			case FUNC_ERROR:
				CloseProgram(TRUE, NULL);
				break;
			}
			break;
		case FUNC_ERROR:
			CloseProgram(TRUE, NULL);
			break;
		}
		break;
	case FUNC_ERROR:
		CloseProgram(FALSE, NULL);
		break;
	}

	// Штатное закрытие программы
	CloseProgram(TRUE, &listen);
	return 0;
}

// Запуск сетевой подсистемы Windows
int InitNetworkSubsystem(WSADATA& wsaData) {
	// Инициализация сетевой подсистемы
	int errStateCode;
	const int sockVersion = 2;
	errStateCode = WSAStartup(MAKEWORD(sockVersion, sockVersion), &wsaData);

	// Проверка на ошибки и перехват их возникновения
	if (errStateCode != 0) {
		DebugInfo(CurFunc);
		HandleError(INIT_NETSUBSYSTEM_ERROR, errStateCode, NULL);
		return FUNC_ERROR;
	}
	else if (LOBYTE(wsaData.wVersion) != sockVersion ||
		HIBYTE(wsaData.wVersion) != sockVersion) {
		DebugInfo(CurFunc);
		errStateCode = WSAVERNOTFOUND;
		HandleError(INIT_NETSUBSYSTEM_ERROR, errStateCode, NULL);
		return FUNC_ERROR;
	}
	else return FUNC_SUCCESS;
}

// Проверка переданных программе параметров
int CheckParams(int argc, TCHAR* argv[], IN_ADDR& IPtoNum) {
	int errStateCode;
	// Проверка на кол-во параметров
	if (argc < 2) {
		DebugInfo(CurFunc);
		errStateCode = PARAMFEWPASSED;
		HandleError(BAD_ARGS_ERROR, errStateCode, NULL);
		return FUNC_ERROR;
	}
	// Проверка на корректность введённого IP/DNS адреса
	else if (CorrectIP_DNS(argv[1], IPtoNum) == FUNC_ERROR) {
		// Вызов обработчика ошибок происходит внутри
		// функции CorrectIP_DNS
		return FUNC_ERROR;
	}
	// Пройдены все проверки
	else {
		/// TODO: Доделать, если будет несколько флагов
		return FUNC_SUCCESS;
	}
}

// Проверка корректности написания IP, 
int CorrectIP_DNS(TCHAR* IP, IN_ADDR& IPtoNum) {
	int errStateCode;
	errStateCode = inet_pton(AF_INET, IP, &IPtoNum);
	if (errStateCode == 0) {
		ADDRINFO* result = NULL;
		ADDRINFO hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_RAW;
		hints.ai_protocol = IPPROTO_ICMP;
		errStateCode = getaddrinfo(IP, NULL, &hints, &result);
		if (errStateCode != 0) {
			DebugInfo(CurFunc);
			HandleError(BAD_ARGS_ERROR, errStateCode, NULL);
			return FUNC_ERROR;
		}
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
	int errStateCode;
	listen = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	// Обработка ошибок, если они есть
	if (listen == INVALID_SOCKET || listen == NULL) {
		DebugInfo(CurFunc);
		errStateCode = WSAGetLastError();
		HandleError(INIT_SOCKET_ERROR, errStateCode, NULL);
		return FUNC_ERROR;
	}

	// Полученный сокет будет принимать данные
	errStateCode = setsockopt(listen, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	// Обработка ошибок, если они есть
	if (errStateCode != 0) {
		DebugInfo(CurFunc);
		HandleError(INIT_SOCKET_ERROR, errStateCode, NULL);
		return FUNC_ERROR;
	}
	return FUNC_SUCCESS;
}

// Формируем эхо-запрос и посылаем его удалённому устройству
int SendRequest(SOCKADDR_IN& remote, SOCKET& listen, WORD SeqNum,
	clock_t& start, STAT& net_stat, string& ip_str) {
	try {
		// Вывод первой строки об обмене данными
		if (SeqNum == 0)
			cout << TEXT("Обмен пакетами с узлом ") << ip_str.c_str() << TEXT(":\n");

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
		int errStateCode;
		errStateCode = sendto(listen, (char*)&req, sizeof(req), 0,
			(sockaddr*)&remote, sizeof(remote));
		start = clock();

		// Проверка на наличие ошибок при отправлении запроса
		if (errStateCode == SOCKET_ERROR) {
			DebugInfo(CurFunc);
			errStateCode = WSAGetLastError();
			HandleError(SEND_REQUEST_ERROR, errStateCode, NULL);
			return FUNC_ERROR;
		}
		// Пишем в лог, что отправили эхо-запрос
		else {
			string msg = TEXT("На ") + ip_str + TEXT(" отправлено ")
				+ toStr(DATA_SIZE) + TEXT(" байт.\n");
			net_stat.sended++;
			WriteMessage(INFO_MESSAGE, &msg);
			return FUNC_SUCCESS;
		}
	}
	// Отлавливаем все возможные ошибки в ходе отправки запроса
	catch (std::exception& e) {
		DebugInfo(CurFunc);
		HandleError(SEND_REQUEST_ERROR, NULL, e.what());
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
		string info;

		// Получаем эхо-ответ
		if (recvfrom(listen, buffer, BUFFER_SIZE, 0, (sockaddr*)&local, &local_size) == SOCKET_ERROR) {
			int errStateCode;
			errStateCode = WSAGetLastError();
			if (errStateCode == WSAETIMEDOUT) timeout = true;
			else {
				DebugInfo(CurFunc);
				HandleError(GET_REPLY_ERROR, errStateCode, NULL);
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
				info = TEXT("Получен ответ от ") + ip_str
					+ TEXT(": число байт=") + toStr(DATA_SIZE) + TEXT(" время=")
					+ toStr(time) + TEXT("мс TTL=") + toStr(ttl) + TEXT(".\n");
				WriteMessage(INFO_MESSAGE, &info);
				cout << info.c_str();
			}
			// Иначе выводим данные о ложном пакете в лог и консоль
			else {
				net_stat.lost++;
				info = TEXT("Принят ложный пакет.\n");
				WriteMessage(INFO_MESSAGE, &info);
				cout << info.c_str();
			}
		}
		// Если время ожидания истекло
		else {
			// Выводим в консоль и лог информацию о таймауте
			info = TEXT("Узел: ") + ip_str + TEXT(".\nПревышен интервал ожидания запроса.\n");
			WriteMessage(INFO_MESSAGE, &info);
			cout << info.c_str();
		}
		return FUNC_SUCCESS;
	}
	// Была получена ошибка во время выполнения
	catch (std::exception& e) {
		DebugInfo(CurFunc);
		HandleError(GET_REPLY_ERROR, NULL, e.what());
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
		string info_str = TEXT("Пакетов: отправлено = ") + toStr(net_stat.sended)
			+ TEXT(", получено = ") + toStr(net_stat.received)
			+ TEXT(", потеряно = ") + toStr(net_stat.lost) + TEXT("    (")
			+ toStr(percLost) + TEXT(" % потерь)\n");

		// Выводим сформированную строку в лог и консоль
		WriteMessage(INFO_MESSAGE, &info_str);
		cout << info_str.c_str();

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
				else if (elem > max) max = elem;
				avg += elem;
			}
			avg = avg / vecSize;

			// Выводим информацию в лог и консоль
			cout << TEXT("Приблизительное время приёма-передачи:\n    ");
			info_str = TEXT("мин=") + toStr(min) + TEXT("мс, макс=")
				+ toStr(max) + TEXT("мс, среднее=") + toStr(avg) + TEXT("мс\n");
			cout << info_str.c_str();
			info_str = TEXT("Статистика приёма-передачи пакетов: ") + info_str;
			WriteMessage(INFO_MESSAGE, &info_str);
		}
		else cout << endl;
		return FUNC_SUCCESS;
	}
	// Была получена ошибка во время выполнения
	catch (std::exception& e) {
		DebugInfo(CurFunc);
		HandleError(STATISTICS_ERROR, NULL, e.what());
		return FUNC_ERROR;
	}
}