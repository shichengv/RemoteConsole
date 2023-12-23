#include "framework.h"
#include "Console.h"
#include "WrapperServerSocket.h"

#define SERVER_LOG			TEXT("..\\x64\\Debug\\server_log.txt")

extern HWND hwndMainWnd;
extern HWND hwndDebugOutput;
extern TCHAR szTime[SZTIMELEN];
extern SYSTEMTIME st;

static HANDLE hServerLogFile;

DWORD dwBytesWritten;
DWORD dwBytesToWrite;

static TCHAR Buffer[8192];
extern TCHAR DebugConsoleBuffer[OUTPUTSIZE];

void UpdateServerLogFile(const TCHAR szMsg[]) {

	GetSystemTime(&st);
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, NULL, szTime, SZTIMELEN);
	_stprintf_s(Buffer, TEXT("%s\t%s...\n"), szTime, szMsg);
	_tcsncat_s(DebugConsoleBuffer, Buffer, OUTPUTSIZE - _tcslen(DebugConsoleBuffer) - 1);
	
	SetWindowText(hwndDebugOutput, DebugConsoleBuffer);
	//SendMessage(hwndDebugOutput, WM_SETTEXT, 0, (LPARAM)Buffer);

	//dwBytesToWrite = (DWORD)_tcsclen(Buffer) * sizeof(TCHAR);
	//BOOL bErrorFlag = WriteFile(hServerLogFile, Buffer, dwBytesToWrite, &dwBytesWritten, NULL);
	//if (FALSE == bErrorFlag)
	//	MessageBox(hwndMainWnd, TEXT("Terminal failure: Unable to write to file."), TEXT("WriteServerLogFile"), MB_OK);
	//else {
	//	if (dwBytesWritten != dwBytesToWrite)
	//	{
	//		// This is an error because a synchronous write that results in
	//		// success (WriteFile returns TRUE) should write all data as
	//		// requested. This would not necessarily be the case for
	//		// asynchronous writes.
	//		MessageBox(hwndMainWnd, TEXT("Error: dwBytesWritten != dwBytesToWrite"), TEXT("WriteServerLogFile"), MB_OK);
	//		
	//	}
	//	else
	//	{
	//		FlushFileBuffers(hServerLogFile);
	//		return;
	//	}
	//}

}

void DebugSocketLog(const TCHAR* LogStr) {
	
	_stprintf_s(Buffer, TEXT("%s %-5d\n"), LogStr, WSAGetLastError());
	MessageBox(hwndMainWnd, Buffer, TEXT("Socket"), MB_OK);
	UpdateServerLogFile(Buffer);

}

BOOL InitSocket() {
	WSADATA wsaData;
	
	hServerLogFile = CreateFile(
		SERVER_LOG,             // name of the write
		GENERIC_WRITE,          // open for writing
		FILE_SHARE_READ,        // read share
		NULL,                   // default security
		CREATE_ALWAYS,          // create new file only
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);                  // no attr. template

	if (hServerLogFile == INVALID_HANDLE_VALUE)
	{
		_stprintf_s(Buffer, TEXT("Terminal failure: Unable to open file \"%s\" for write.\n"), SERVER_LOG);
		MessageBox(hwndMainWnd, Buffer, TEXT("CreateFile"), MB_OK);
		return 0;
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		DebugSocketLog(TEXT("InitSocket() -> WSAStartup() error"));
		return FALSE;
	}
	return TRUE;
}

void CleanSocket() {
	WSACleanup();
	if (hServerLogFile != INVALID_HANDLE_VALUE)
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
