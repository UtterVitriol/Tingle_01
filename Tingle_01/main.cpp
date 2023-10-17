#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#ifndef NDEBUG
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define DERR(s, d) fprintf(stderr, "[-]: %s:%d:%s(): %s - %d\n", __FILENAME__, __LINE__, __func__, s, d)
#define DMSG(s) printf("[+]: %s:%d:%s(): %s\n", __FILENAME__, __LINE__, __func__, s)
#else
#define DERR(s,d)
#define DMSG(s)
#endif

void print_help()
{
	puts("./Tingle_01.exe <port>");
}

void print_win()
{
	puts("flag{you_won!}");
}

void vuln(char* buf, int len)
{
	char overflow[16] = { 0 };
	memcpy(overflow, buf, len);
}

int main(int argc, char** argv) {

	if (argc != 2)
	{
		print_help();
		return 1;
	}

	DWORD iResult = 0;
	DWORD dwError = 0;
	WSADATA wsaData = { 0 };

	SOCKET sock = INVALID_SOCKET;
	SOCKET sClient = INVALID_SOCKET;
	struct addrinfo* result = NULL;
	struct addrinfo hints;

	struct sockaddr_in sAddrIn = { 0 };
	int len = sizeof(sAddrIn);

	PCHAR pcBuf = (PCHAR)HeapAlloc(GetProcessHeap(), 0, 4098);
	WSABUF wsaBuf = { 4098, pcBuf };

	std::string yousaid = "You said: ";
	CHAR goodbye[] = "Goodbye.";

	DWORD dwRecvd = 0;
	DWORD dwFlags = 0;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0)
	{
		dwError = GetLastError();
		DERR(std::system_category().message(dwError).c_str(), dwError);
		return iResult;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	iResult = getaddrinfo("127.0.0.1", argv[1], &hints, &result);
	if (iResult != 0) {
		dwError = GetLastError();
		print_help();
		return dwError;
	}

	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock == INVALID_SOCKET) {

		dwError = GetLastError();
		DERR(std::system_category().message(dwError).c_str(), dwError);
		freeaddrinfo(result);
		return dwError;
	}

	iResult = bind(sock, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		freeaddrinfo(result);
		dwError = GetLastError();
		DERR(std::system_category().message(dwError).c_str(), dwError);
		return dwError;
	}


	iResult = listen(sock, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		dwError = GetLastError();
		DERR(std::system_category().message(dwError).c_str(), dwError);
		return dwError;
	}


	iResult = getsockname(sock, (sockaddr*)&sAddrIn, &len);
	if (iResult != 0)
	{
		dwError = GetLastError();
		DERR(std::system_category().message(dwError).c_str(), dwError);
		return dwError;
	}

	printf("Listening on port: %d\n", ntohs(sAddrIn.sin_port));

	freeaddrinfo(result);

	sClient = accept(sock, NULL, NULL);

	if (INVALID_SOCKET == sClient)
	{
		dwError = GetLastError();
		DERR(std::system_category().message(dwError).c_str(), dwError);
		return dwError;
	}

	iResult = WSARecv(sClient, &wsaBuf, 1, &dwRecvd, &dwFlags, 0, 0);
	if (iResult < 0)
	{
		dwError = GetLastError();
		DERR(std::system_category().message(dwError).c_str(), dwError);
		goto END;

	}

	vuln(wsaBuf.buf, dwRecvd);

	yousaid.append(wsaBuf.buf);

	iResult = send(sClient, yousaid.c_str(), yousaid.size(), 0);
	if (iResult <= 0)
	{
		dwError = GetLastError();
		DERR(std::system_category().message(dwError).c_str(), dwError);
		goto END;
	}


	iResult = send(sClient, goodbye, sizeof(goodbye), 0);
	if (iResult <= 0)
	{
		dwError = GetLastError();
		DERR(std::system_category().message(dwError).c_str(), dwError);
		goto END;
	}

END:
	closesocket(sock);
	closesocket(sClient);

	WSACleanup();

	return dwError;
}