// Console.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Console.h"
#include "WrapperSocket.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "ws2_32.lib")

HINSTANCE HInstance;
HFONT hFont;

HWND hwndMainWnd;
HWND hwndListBox;
HWND hwndRemoteConsole;
HWND hwndRemoteConsoleEdit;
HWND hwndLConsole;
HWND hwndLConsoleEdit;

HMENU hMenuListBoxItem;	// Pop-up menu handle

SOCKET ServerSocket = INVALID_SOCKET;

SYSTEMTIME st;
TCHAR szTime[SZTIMELEN];

TCHAR	szAppName[] = TEXT("MainWnd");
TCHAR	szConsoleClass[] = TEXT("Console");

extern LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK ListWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, 
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

extern LRESULT CALLBACK ConsoleWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       iCmdShow)
{
	MSG				msg;
	WNDCLASS		wndclass = {};

	HInstance = hInstance;

	InitSocket();

	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_HREDRAW;
	wndclass.lpfnWndProc = MainWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadIcon(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;


	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}


	wndclass.lpfnWndProc = ConsoleWndProc;
	wndclass.cbWndExtra = sizeof(long);
	wndclass.hIcon = NULL;
	wndclass.lpszClassName = szConsoleClass;

	RegisterClass(&wndclass);

	hwndMainWnd = CreateWindow(szAppName,
		TEXT("Main Window"),
		WS_OVERLAPPEDWINDOW | WS_BORDER,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL,
		);
	if (hwndMainWnd == NULL)
	{
		MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwndMainWnd, iCmdShow);
	UpdateWindow(hwndMainWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
