#include "winsock2.h"
#include "iphlpapi.h"
#include "IcmpAPI.h"
#include <string>
#include <iostream>
#pragma warning(disable : 4996)

using namespace std;

#define TIME_OUT 4000
#define REQUESTS 4

unsigned long ipaddr = INADDR_NONE;     // Пингуемый адрес в формате,
										// подходящем для структуры IN_ADDR
char message[32] = { "Ping programm" }; // Данные для отправки
DWORD responceCount = 0;                // Количество ответов, если = 0, ответ не был получен
DWORD responceTotalSize = 0;            // Размер буфера ответа
void* responce = NULL;					// Указатель на буфер ответа
HANDLE icmpH;							// Дескриптор

/// <summary>
/// Функция обработки отсутствия сигнала.
/// Определяет тип ошибки.
/// </summary>
/// <param name="addr"></param>
void notResponce(char* addr) {
	cout << "Address " + (string)addr + " is not responding. " "\n";
	DWORD errCode = GetLastError();
	switch (errCode)
	{
	case ERROR_INSUFFICIENT_BUFFER:
		cout << "responceTotalSize is too small\n";
		break;
	case ERROR_INVALID_PARAMETER:
		cout << "Invalid parameter\n";
		break;
	case ERROR_NOT_ENOUGH_MEMORY:
		cout << "Not enough memory to end operation\n";
		break;
	case ERROR_NOT_SUPPORTED:
		cout << "IPv4 not supported on local computer\n";
		break;
	case IP_BUF_TOO_SMALL:
		cout << "ResponceBuffer was too small\n";
		break;
	default:
		cout << "The response timeout has expired\n";
		break;
	}
}

void responceProcessing(char* addr) {
	PICMP_ECHO_REPLY echo = (PICMP_ECHO_REPLY)responce;					// Ответ необходимо преобразовать в структуру PICMP_ECHO_REPLY
	in_addr in_addr;													// Через эту структуру числовое представление
	in_addr.S_un.S_addr = echo->Address;								// Запись эхо-адреса для дальнейшего преобразования
																		// адреса преобразуется в строковое

	cout << "Ping address: " + (string)addr + "\n";
	cout << "Round trip time: " + to_string(echo->RoundTripTime) + "\n";
	cout << "Replying address: " + (string)inet_ntoa(in_addr) + "\n";
	cout << "Echo status: " + to_string(echo->Status) + "\n \n";
	Sleep(1000);
}

int main(int argc, char* args[]) {
	int respondCount = 0;
	int notRespondCount = 0;
	if (argc < 2)
	{
		cout << "IP expected";
		return 1;
	}
	char* address = args[1];
	ipaddr = inet_addr(address);			// Перевод строчного представления адреса в
											// требуемый формат. Ожмдается запись вида 8.8.8.8
											// TODO: предварительно проверять, введён DNS или IP, если DNS - получить адрес по имени
	icmpH = IcmpCreateFile();				// Открытие дескриптора

	responceTotalSize = sizeof(ICMP_ECHO_REPLY) + sizeof(message);			// Размер буфера ответа
																			// Должен быть достаточно большим для структуры
																			// ICMP_ECHO_REPLY + размер сообщения запроса
	responce = (void*)malloc(responceTotalSize);							// Выделяем необходимую память, на которую указывает указатель

	for (size_t i = 0; i < REQUESTS; i++)
	{
		if (IcmpSendEcho(icmpH, ipaddr, message,
			sizeof(message), NULL, responce, responceTotalSize, TIME_OUT) == 0) // Посылка запроса
																				// 0 - ответ не получен
		{
			notResponce(address);
			notRespondCount++;
		}
		else																	// ответ получен
		{
			responceProcessing(address);
			respondCount++;
		}
	}
	cout << to_string(respondCount) + " requests completed successfully\n" +
		to_string(notRespondCount) + " requests were executed with an error\ntotal requests = " + to_string(REQUESTS) + "\n";
	free(responce); // Освобождаем память
	return 0;
}