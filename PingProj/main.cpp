#include "main.hpp"
#include "logger.hpp"

#pragma comment(lib, "Ws2_32.lib")

// ������� ������� ���������
int main(int argc, TCHAR* argv[]) {
	setlocale(LC_ALL, dLocale);
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
	timeout = 1000;

	// ������� �����
	switch (InitLogger()) {
	case FUNC_SUCCESS:
		switch (InitNetworkSubsystem(wsaData)) {
		case FUNC_SUCCESS:
			switch (CheckParams(argc, argv, IP_Address)) {
			case FUNC_SUCCESS:
				switch (InitSocks(remote, bind, local, listen, IP_Address, port, timeout)) {
				case FUNC_SUCCESS:
					cout << 5 << endl;
					break;
				case FUNC_ERROR:
					/// TODO: �������� ����������� ������
					cout << 4 << endl;
					break;
				}
				break;
			case FUNC_ERROR:
				/// TODO: �������� ����������� ������
				cout << 3 << endl;
				break;
			}
			break;
		case FUNC_ERROR:
			/// TODO: �������� ����������� ������
			cout << 2 << endl;
			break;
		}
		break;
	case FUNC_ERROR:
		/// TODO: �������� ����������� ������
		cout << 1 << endl;
		break;
	}




	system("pause");
	return 0;
}

// ������ ������� ���������� Windows
int InitNetworkSubsystem(WSADATA& wsaData) {
	// ������������� ������� ����������
	int errorStateCode;
	const int sockVersion = 2;
	errorStateCode = WSAStartup(MAKEWORD(sockVersion, sockVersion), &wsaData);

	// �������� �� ������
	/// TODO: ��������� ��� � ������
	if (errorStateCode != 0) {
		cout << TEXT("������ ������������� WinSock #");
		cout << errorStateCode << TEXT(":\n");
		switch (errorStateCode) {
		case WSASYSNOTREADY:
			cout << TEXT("������� ������� ���������� �� ������ � �������� ��������������.\n");
			break;
		case WSAVERNOTSUPPORTED:
			cout << TEXT("����������� ������ ������� �� �������������� ������ ����������� ������� Windows.\n");
			break;
		case WSAEINPROGRESS:
			cout << TEXT("����������� �������� ���������� �������.\n");
			break;
		case WSAEPROCLIM:
			cout << TEXT("���������� ����������� �� ���������� �����, �������������� ����������� ������� Windows.\n");
			break;
		case WSAEFAULT:
			cout << TEXT("������� ������� ������������ ���������.\n");
			break;
		default:
			cout << TEXT("������� ���������� ������� ������������ ��������.\n");
			break;
		}
		return FUNC_ERROR;
	}
	/// TODO: ��������� ��� � ������
	else if (LOBYTE(wsaData.wVersion) != sockVersion ||
		HIBYTE(wsaData.wVersion) != sockVersion) {
		cout << TEXT("�� ������� ��������� ������ Winsock.dll.\n");
		WSACleanup();
		return FUNC_ERROR;
	}
	else return FUNC_SUCCESS;
}

// �������� ���������� ��������� ����������
int CheckParams(int argc, TCHAR* argv[], IN_ADDR& IPtoNum) {
	// �������� �� ���-�� ����������
	if (argc < 2) {
		/// TODO:
		// ��� ��� ������� � ������������ ������
		return FUNC_ERROR;
	}
	// �������� �� ������������ ��������� IP/DNS ������
	else if (CorrectIP_DNS(argv[1], IPtoNum) == FUNC_ERROR) {
		/// TODO:
		// ��� ��� ������� � ������������ ������
		return FUNC_ERROR;
	}
	//
	else {
		/// TODO: ��������, ���� ����� ��������� ������
		/*
		// ���� ������ ���� ����������, ������������ ��
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

// �������� ������������ ��������� IP, 
int CorrectIP_DNS(TCHAR* IP, IN_ADDR& IPtoNum) {
	int erStat;	
	erStat = inet_pton(AF_INET, IP, &IPtoNum);
	if (IPtoNum.S_un.S_addr == INADDR_NONE) {
		//hostent* hp = NULL;
		//hp = gethostbyname(IP);
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
			/// TODO: �������� ����������� ������
			// ��� ��� ������� � ������������ ������
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

// ������������� ������� ���������� � ������
int InitSocks(SOCKADDR_IN& remote, SOCKADDR_IN& bind, SOCKADDR_IN& local, SOCKET& listen,
	IN_ADDR& ip_num, WORD& port, DWORD& timeout) {
	// ������ ����� � ��������� �� ������� ������ ��� ��������
	int errorStateCode;
	listen = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (listen == INVALID_SOCKET || listen == NULL) {
		/// TODO: �������� ����������� ������
		// ��� ��� ������� � ������������ ������
		errorStateCode = WSAGetLastError();
		cout << TEXT("������ �������� ������: # ") << errorStateCode << TEXT(":\n");

		/// TODO: ������� � ��������� �������
		// ��� ��� ������� � ������������ ������
		closesocket(listen);
		WSACleanup();
		return FUNC_ERROR;
	}

	// ���������� ����� ����� ��������� ������
	errorStateCode = setsockopt(listen, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	if (errorStateCode != 0) {
		/// TODO: ������� � ��������� �������
		// ��� ��� ������� � ������������ ������
		return FUNC_ERROR;
	}

	// ��������� �������
	remote.sin_addr = ip_num;
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);

	bind.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind.sin_family = AF_INET;
	bind.sin_port = htons(port);

	local.sin_family = AF_INET;

	closesocket(listen);
	WSACleanup();
	return FUNC_SUCCESS;
}









#ifdef _DEBUG 
#define num1
#else
#define num2
#endif
