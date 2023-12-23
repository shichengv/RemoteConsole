#include "framework.h"
#include "Client.h"
#include "WrapperClientSocket.h"

#define CLIENT_LOG			TEXT("client_log.txt")

extern SYSTEMTIME st;
extern TCHAR szTime[SZTIMELEN];

static TCHAR Buffer[8192];
static DWORD dwBytesToWrite;
static DWORD dwBytesWritten;

HANDLE hClientLogFile;

void UpdateClientLogFile(const TCHAR szMsg[]) {

	if (hClientLogFile == INVALID_HANDLE_VALUE)
		return;

	GetSystemTime(&st);
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, szTime, SZTIMELEN);
	_stprintf_s(Buffer, TEXT("%s\t%s...\n"), szTime, szMsg);
	dwBytesToWrite = _tcsclen(Buffer) * sizeof(TCHAR);
	BOOL bErrorFlag = WriteFile(hClientLogFile, Buffer, dwBytesToWrite, &dwBytesWritten, NULL);

	FlushFileBuffers(hClientLogFile);

}

void DebugSocketLog(const TCHAR* LogStr) {

	_stprintf_s(Buffer, TEXT("%s %-5d\n"), LogStr, WSAGetLastError());
	if (hClientLogFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	UpdateClientLogFile(Buffer);

}

BOOL InitSocket() {
	WSADATA wsaData;

	hClientLogFile = CreateFile(
		CLIENT_LOG,             // name of the write
		GENERIC_WRITE,          // open for writing
		0,                      // do not share
		NULL,                   // default security
		CREATE_NEW,             // create new file only
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);                  // no attr. template


	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		DebugSocketLog(TEXT("InitSocket() -> WSAStartup() error"));
		return FALSE;
	}
	return TRUE;


}

void CleanSocket() {
	WSACleanup();

	if (hClientLogFile != INVALID_HANDLE_VALUE)
		CloseHandle(hClientLogFile);
}

SOCKET OpenClientSocket(PCSTR HostName, PCSTR Port) {
	SOCKET clientfd = INVALID_SOCKET;
	struct addrinfo hints, * listp, * p;
	int iResult;

	ZeroMemory(&hints, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_flags |= AI_ADDRCONFIG;
	hints.ai_flags |= AI_NUMERICSERV;

	iResult = getaddrinfo(HostName, Port, &hints, &listp);
	if (iResult != 0)
	{
		DebugSocketLog(TEXT("OpenClientSocket() -> getaddrinfo() error"));
		return clientfd;
	}

	// Walk the list for one that we can bind to */
	for (p = listp; p; p = p->ai_next)
	{
		if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			continue;
		if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
			break;
		closesocket(clientfd);
	}

	freeaddrinfo(listp);
	if (!p)
		return INVALID_SOCKET;
	else
		return clientfd;
}