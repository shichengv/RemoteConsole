#include "framework.h"
#include "Console.h"
#include "Server.h"
#include "WrapperSocket.h"


extern SOCKET ServerSocket;
extern SYSTEMTIME st;
extern TCHAR szTime[SZTIMELEN];
extern HWND hwndMainWnd;

HANDLE hServerLogFile;

static int key = 0;
static TCHAR Buffer[8192];

HashTable htClients;


DWORD WINAPI ListConnectedClients(LPVOID lpParameter) {


	HWND hwndChild = (HWND)lpParameter;
	DWORD dwThreadId = GetCurrentThreadId();

	/* Status */
	int iResult;

	/*	��ʱHost	*/
	int cSize;
	char clientHost[NI_MAXHOST];
	char clientService[NI_MAXSERV];
	TCHAR wClientHost[HOSTNAMELEN];
	TCHAR wClientService[HOSTNAMELEN];

	/*	�ͻ�Socket	*/
	SOCKET ClientSocket = INVALID_SOCKET;
	/*  �ͻ���Ϣ	*/
	struct sockaddr_storage clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	/*	ָ��ͻ�ʵ�����	*/
	Client* pCAcceptClient;

	TCHAR szFileName[SZTIMELEN] = { };

	_stprintf_s(szFileName, TEXT("server_log.txt"));

	hServerLogFile = CreateFile(
		szFileName,             // name of the write
		GENERIC_WRITE,          // open for writing
		0,                      // do not share
		NULL,                   // default security
		CREATE_NEW,             // create new file only
		FILE_ATTRIBUTE_NORMAL,  // normal file
		NULL);                  // no attr. template

	if (hServerLogFile == INVALID_HANDLE_VALUE)
	{
		_stprintf_s(Buffer, TEXT("Terminal failure: Unable to open file \"%s\" for write.\n"), szFileName);
		MessageBox(hwndMainWnd, Buffer, TEXT("CreateFile"), MB_OK);
		return 0;
	}

	UpdateServerLogFile(hServerLogFile, TEXT("��ʼ����"), &st);

	ServerSocket = OpenListenSocket(DEFAULT_PORT);
	while (1)
	{
		ClientSocket = accept(ServerSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
		if (ClientSocket == INVALID_SOCKET) {
			DebugSocketLog(TEXT("Accept Faild:"));
			continue;
		}

		/* ��ȡ�ͻ����������ֲ�ת��Ϊ���ַ�	*/
		iResult = getnameinfo((struct sockaddr*)&clientAddr, clientAddrLen, clientHost, NI_MAXHOST, 
			clientService, NI_MAXSERV, 0);
		if (iResult != 0)
		{
			_stprintf_s(Buffer, TEXT("getnameinfo failed: %d\n"), iResult);
			DebugSocketLog(Buffer);
		}
		cSize = strlen(clientHost) + 1;
		MultiByteToWideChar(CP_UTF8, 0, clientHost, cSize, wClientHost, HOSTNAMELEN);
		cSize = strlen(clientService) + 1;
		MultiByteToWideChar(CP_UTF8, 0, clientService, cSize, wClientService, HOSTNAMELEN);
		_stprintf_s(Buffer, TEXT("%s:%s"), wClientHost, wClientService);

		/*	�½�һ�����󣬲��洢��HashTable��	*/
		pCAcceptClient = new Client;
		pCAcceptClient->Initilize(ClientSocket, ++key, Buffer, TEXT(" "), pCAcceptClient);
		htClients.Insert(key, pCAcceptClient);
		_stprintf_s(Buffer, TEXT("�������ӣ�%s"), Buffer);
		DebugSocketLog(Buffer);
		SendMessage(hwndChild, LB_ADDSTRING, 0, (LPARAM)Buffer);

	}

	return dwThreadId;
}