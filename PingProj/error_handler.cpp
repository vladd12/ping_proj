#include "main.hpp"

int GlobalErrorCode = 0;

// Функция, перехватывающая ошибки
void HandleError(int majorErrCode, int minorErrCode, const char* const errStr) {
	try {
		// Получаем строку
		string errString;
		if (errStr != NULL) errString = Convert(errStr);
		GlobalErrorCode = majorErrCode;

		// Выбираем, что делать
		switch (majorErrCode) {
		case INIT_NETSUBSYSTEM_ERROR:
			errString = TEXT("Init Network Subsystem Failure #") + toStr(minorErrCode) + TEXT(": ");
			switch (minorErrCode) {
			case WSASYSNOTREADY:
				errString += TEXT("Базовая сетевая подсистема не готова к сетевому взаимодействию.\n");
				break;
			case WSAVERNOTSUPPORTED:
				errString += TEXT("Запрошенная версия сокетов не поддерживается данной реализацией сокетов Windows.\n");
				break;
			case WSAVERNOTFOUND:
				errString += TEXT("Не найдена указанная версия Winsock.dll.\n");
				break;
			case WSAEINPROGRESS:
				errString += TEXT("Выполняется операция блокировки сокетов.\n");
				break;
			case WSAEPROCLIM:
				errString += TEXT("Достигнуто ограничение на количество задач, поддерживаемых реализацией сокетов Windows.\n");
				break;
			case WSAEFAULT:
				errString += TEXT("Функции передан недопустимый указатель.\n");
				break;
			default:
				errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
				break;
			}
			break;
		case BAD_ARGS_ERROR:
			errString = TEXT("Bad Arguments Passed #") + toStr(minorErrCode) + TEXT(": ");
			switch (minorErrCode) {
			case PARAMFEWPASSED:
				errString += TEXT("Передано слишком маленькое число параметров.\n");
				UsageOutput();
				break;
			case WSANO_RECOVERY:
				errString += TEXT("Произошла неисправимая ошибка в разрешении имён.\n");
				break;
			case WSA_NOT_ENOUGH_MEMORY:
				errString += TEXT("Произошёл сбой выделения памяти.\n");
				break;
			case WSAHOST_NOT_FOUND:
				errString = errString + TEXT("Удалённый узел с указанным именем не найден.\n");
				break;
			case WSATYPE_NOT_FOUND:
				errString += TEXT("Указанная служба не поддерживается указанным типом сокетов.\n");
				break;
			case WSAESOCKTNOSUPPORT:
				errString += TEXT("Указанный сокет не поддерживается функцией getaddrinfo.\n");
				break;
			default:
				errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
				break;
			}
			break;
		case INIT_SOCKET_ERROR:
			errString = TEXT("Init Socket Failure #") + toStr(minorErrCode) + TEXT(": ");
			switch (minorErrCode) {
			case WSAENETDOWN:
				errString += TEXT("Произошёл сбой сетевой подсистемы или отсутствует подключение к сети Интернет.\n");
				break;
			case WSAEINPROGRESS:
				errString += TEXT("Выполняется блокирующий вызов сокетов, или поставщик услуг Интернет обрабатывает функцию обратного вызова.\n");
				break;
			case WSAEMFILE:
				errString += TEXT("Не осталось доступных дескрипторов сокетов.\n");
				break;
			case WSAEINVALIDPROVIDER:
				errString += TEXT("Поставщик услуг Интернет вернул сокет, отличный от указанного.\n");
				break;
			case WSAEINVALIDPROCTABLE:
				errString += TEXT("Поставщик услуг Интернет вернул неверную или неполную таблицу процедур.\n");
				break;
			case WSAENOBUFS:
				errString += TEXT("Буферное пространство недоступно. Сокет не может быть создан.\n");
				break;
			case WSAEPROVIDERFAILEDINIT:
				errString += TEXT("Поставщику услуг Интернет не удалось выполнить инициализацию.\n");
				break;
			case WSAENOTSOCK:
				errString += TEXT("Полученный дескриптор не является сокетом.\n");
				break;
			default:
				errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
				break;
			}
			break;
		case SEND_REQUEST_ERROR:
			if (minorErrCode != NULL) {
				errString = TEXT("Send Echo Request Failure #") + toStr(minorErrCode) + TEXT(": ");
				switch (minorErrCode) {
				case WSAENETDOWN:
					errString += TEXT("Произошёл сбой сетевой подсистемы или отсутствует подключение к сети Интернет.\n");
					break;
				case WSAEINPROGRESS:
					errString += TEXT("Выполняется блокирующий вызов сокетов, или поставщик услуг Интернет обрабатывает функцию обратного вызова.\n");
					break;
				case WSAEACCES:
					errString += TEXT("Запрошенный адрес является широковещательным.\n");
					break;
				case WSAENETRESET:
					errString += TEXT("Соединение было разорвано во время выполнения операции.\n");
					break;
				case WSAENOBUFS:
					errString += TEXT("Буферное пространство недоступно.\n");
					break;
				case WSAEHOSTUNREACH:
					errString += TEXT("Невозможно связаться с удалённым хостом.\n");
					break;
				case WSAECONNABORTED:
					errString += TEXT("Виртуальный канал был прерван из-за тайм-аута или другого сбоя.\n");
					break;
				case WSAEADDRNOTAVAIL:
					errString += TEXT("Удалённый адрес не является допустимым адресом.\n");
					break;
				case WSAENETUNREACH:
					errString += TEXT("Невозможно подключиться к сети Интернет.\n");
					break;
				case WSAETIMEDOUT:
					errString += TEXT("Соединение было прервано из-за сбоя сети или из-за того, что система на другом конце вышла из строя без уведомления.\n");
					break;
				default:
					errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
					break;
				}
			}
			break;
		case GET_REPLY_ERROR:
			if (minorErrCode != NULL) {
				errString = TEXT("Get Echo Reply Failure #") + toStr(minorErrCode) + TEXT(": ");
				switch (minorErrCode) {
				case WSAENETDOWN:
					errString += TEXT("Произошёл сбой сетевой подсистемы или отсутствует подключение к сети Интернет.\n");
					break;
				case WSAEINPROGRESS:
					errString += TEXT("Выполняется блокирующий вызов сокетов, или поставщик услуг Интернет обрабатывает функцию обратного вызова.\n");
					break;
				case WSAENETRESET:
					errString += TEXT("Сетевая подсистема была сброшена.\n");
					break;
				case WSAECONNRESET:
					errString += TEXT("Соединение было сброшено удалённой стороной.\n");
					break;
				default:
					errString += TEXT("Сетевая подсистема вернула недопустимое значение.\n");
					break;
				}
			}
			break;
		default:
			break;
		}

		// Запись в лог об ошибке, или вывод в консоль, если лог недоступен
		BOOL state = RedColorText();
		if (majorErrCode != INIT_LOGGER_ERROR && majorErrCode != WRITE_LOG_ERROR) {
			WriteMessage(ERROR_MESSAGE, &errString);
			cout << errString.c_str();
		}
		else cout << errString.c_str() << endl;
		state = WhiteColorText();
	}

	// Ошибки при работе с ошибками
	catch (exception& e) {
		DebugInfo(CurFunc);
		cout << ConvertCharToWstring(e.what()).c_str() << endl;
		GlobalErrorCode = ERROR_HANDLE_FAILURE;
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
void UsageOutput() {
	cout << TEXT("Запуск программы: ping <IP-адрес>\nили: ping <доменное_имя>\n");
}

// Функция для завершения работы программы
void CloseProgram(BOOL isLoggerGood, SOCKET* sock) {
	if (isLoggerGood == FALSE) exit(GlobalErrorCode);
	else {
		if (sock != NULL) closesocket(*sock);
		WSACleanup();
		WriteMessage(CLOSE_MESSAGE, NULL);
		system("pause");
		exit(GlobalErrorCode);
	}
} 