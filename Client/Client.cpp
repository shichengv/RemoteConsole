// Client.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Client.h"
#include "WrapperClientSocket.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "ws2_32.lib")

SYSTEMTIME st;
TCHAR szTime[SZTIMELEN];

SOCKET ServerSocket = INVALID_SOCKET;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    InitSocket();
    ServerSocket = OpenClientSocket(HOSTNAME, SERVICE);
    while (1)
    {
        Sleep(1000);
    }
    
    
    return 0;
}
