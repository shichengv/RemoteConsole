#include "framework.h"
#include "Console.h"
#include "WrapperSocket.h"

extern HWND hwndMainWnd;
extern TCHAR szTime[SZTIMELEN];
extern SYSTEMTIME st;

extern HANDLE hServerLogFile;

DWORD dwBytesWritten;
DWORD dwBytesToWrite;

static TCHAR Buffer[8192];

void UpdateServerLogFile(HANDLE hFile, const TCHAR szMsg[], SYSTEMTIME* st) {

	GetSystemTime(st);
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, st, NULL, szTime, SZTIMELEN);
	_stprintf_s(Buffer, TEXT("%s\t%s...\n"), szTime, szMsg);
	dwBytesToWrite = _tcsclen(Buffer);
	BOOL bErrorFlag = WriteFile(hServerLogFile, Buffer, dwBytesToWrite, &dwBytesWritten, NULL);
	if (FALSE == bErrorFlag)
		MessageBox(hwndMainWnd, TEXT("Terminal failure: Unable to write to file."), TEXT("WriteServerLogFile"), MB_OK);
	else {
		if (dwBytesWritten != dwBytesToWrite)
		{
			// This is an error because a synchronous write that results in
			// success (WriteFile returns TRUE) should write all data as
			// requested. This would not necessarily be the case for
			// asynchronous writes.
			MessageBox(hwndMainWnd, TEXT("Error: dwBytesWritten != dwBytesToWrite"), TEXT("WriteServerLogFile"), MB_OK);
			
		}
		else
		{
			return;
		}
	}

}

void DebugSocketLog(const TCHAR* LogStr) {
	
	_stprintf_s(Buffer, TEXT("%s %-5d\n"), LogStr, WSAGetLastError());
	UpdateServerLogFile(hServerLogFile, Buffer, &st);

}

BOOL InitSocket() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		DebugSocketLog(TEXT("InitSocket() -> WSAStartup() error"));
		return FALSE;
	}
	return TRUE;
}

void CleanSocket() {
	WSACleanup();
	CloseHandle(hServerLogFile);
}

SOCKET OpenListenSocket(PCSTR Port) {
	struct addrinfo hints, * listp, * p;
	SOCKET listenfd = INVALID_SOCKET;
	int optval = 1;
	int iResult;

	ZeroMemory(&hints, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_flags |= AI_ADDRCONFIG;
	hints.ai_flags |= AI_NUMERICSERV;

	iResult = getaddrinfo(NULL, Port, &hints, &listp);
	if (iResult != 0)
	{
		DebugSocketLog(TEXT("OpenListenSocket() -> getaddrinfo() error"));
		return listenfd;
	}

	// Walk the list for one that we can bind to */
	for (p = listp; p; p = p->ai_next)
	{
		if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			continue;
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
			(const char*)&optval, sizeof(int));
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
			break;
		closesocket(listenfd);
	}

	freeaddrinfo(listp);
	if (!p)
		return INVALID_SOCKET;

	if (listen(listenfd, LISTENQ) < 0)
	{
		closesocket(listenfd);
		return INVALID_SOCKET;
	}
	return listenfd;
}
