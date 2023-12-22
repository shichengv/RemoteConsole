#include "framework.h"
#include "Console.h"
#include "WrapperSocket.h"
#include "Server.h"

extern HFONT hFont;

extern HWND hwndListBox;
extern HWND hwndRemoteConsole;
extern HWND hwndRemoteConsoleEdit;
extern HWND hwndLConsole;
extern HWND hwndLConsoleEdit;

extern HINSTANCE HInstance;

extern HMENU hMenuListBoxItem;	// Pop-up menu handle

extern TCHAR	szConsoleClass[];

extern LRESULT CALLBACK ListWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, 
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
extern DWORD WINAPI ListConnectedClients(LPVOID lpParameter);


extern HashTable htClients;

int cxChar, cxCaps, cyChar;
int iMaxWidth;
int cxClient, cyClient;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	HDC hdc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;

	switch (message)
	{
	case WM_CREATE:
	{
		hdc = GetDC(hwnd);

		hFont = CreateFont(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, GB2312_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("微软雅黑"));
		if (!hFont) {
			MessageBox(hwnd, TEXT("未找到\"微软雅黑\"字体，将使用默认字体。"), TEXT("警告"), MB_OK);
			DeleteObject(hFont);
		}
		SelectObject(hdc, hFont);


		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		iMaxWidth = 40 * cxChar + 22 * cxCaps;
		ReleaseDC(hwnd, hdc);


		// Create a list box control
		hwndListBox = CreateWindow(TEXT("listbox"), NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | LBS_NOTIFY,
			0, 0, 0, 0, hwnd, (HMENU)LIST_ID, HInstance, NULL);
		hwndRemoteConsole = CreateWindow(szConsoleClass, TEXT("远程控制台"),
			WS_CHILDWINDOW | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
			ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_BORDER,
			0, 0, 0, 0,
			hwnd, (HMENU)REMOTECONSOLE_ID, HInstance, NULL);
		hwndLConsole = CreateWindow(szConsoleClass, TEXT("本地控制台"),
			WS_CHILDWINDOW | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
			ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_BORDER,
			0, 0, 0, 0,
			hwnd, (HMENU)LCONSOLE_ID, HInstance, NULL);

		hwndRemoteConsoleEdit = CreateWindowEx(
			WS_EX_CLIENTEDGE, // extended style
			TEXT("EDIT"), // class name
			TEXT(""), // initial text
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, // style
			0, 0, 0, 0,
			hwnd, // parent window handle
			(HMENU)REMOTECONSOLE_EDIT_ID, // control ID
			HInstance,
			NULL // additional data
		);

		hwndLConsoleEdit = CreateWindowEx(
			WS_EX_CLIENTEDGE, // extended style
			TEXT("EDIT"), // class name
			TEXT(""), // initial text
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL, // style
			0, 0, 0, 0,
			hwnd, // parent window handle
			(HMENU)LCONSOLE_EDIT_ID, // control ID
			HInstance,
			NULL // additional data
		);

		SendMessage(hwndListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(hwndRemoteConsoleEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(hwndLConsoleEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

		hMenuListBoxItem = CreatePopupMenu();
		AppendMenu(hMenuListBoxItem, MF_STRING, IDM_DELETE, TEXT("删除该项"));
		AppendMenu(hMenuListBoxItem, MF_STRING, IDM_OPEN, TEXT("打开控制台"));

		SetWindowSubclass(hwndListBox, ListWndProc, 0, 0);
		CreateThread(NULL, 0, ListConnectedClients, (LPVOID)hwndListBox, 0, NULL);
	}
	return 0;

	case WM_SIZE:
	{
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		MoveWindow(hwndListBox, 0, 0, cxClient / 4, cyClient + 6, TRUE);
		MoveWindow(hwndRemoteConsole, cxClient / 4, 0, cxClient / 4 * 3, cyClient / 3 * 2 - cyChar - 10, TRUE);
		MoveWindow(hwndRemoteConsoleEdit, cxClient / 4, cyClient / 3 * 2 - cyChar - 10, cxClient / 4 * 3, cyChar + 10, TRUE);
		MoveWindow(hwndLConsole, cxClient / 4, cyClient / 3 * 2, cxClient / 4 * 3, cyClient / 3 - cyChar - 10, TRUE);
		MoveWindow(hwndLConsoleEdit, cxClient / 4, cyClient - cyChar - 10, cxClient / 4 * 3, cyChar + 10, TRUE);

	}
	return 0;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDM_DELETE:
		{
			int index = SendMessage(hwndListBox, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR)
			{
				SendMessage(hwndListBox, LB_DELETESTRING, index, 0);
			}
		}
		case IDM_OPEN:
		{
			int index = SendMessage(hwndListBox, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR)
			{
				TCHAR buffer[256];
				SendMessage(hwndListBox, LB_GETTEXT, index, (LPARAM)buffer);
				buffer[255] = TEXT('\0');
				SetWindowText(hwndRemoteConsole, buffer);
			}
		}
		default:
			break;
		}
	}
	break;

	case WM_CLOSE:
	{
		DeleteObject(hFont);
		CleanSocket();
		DestroyWindow(hwnd);
	}
	return 0;

	case WM_DESTROY:
	{
		RemoveWindowSubclass(hwndListBox, ListWndProc, 0);
		DeleteObject(hFont);
		CleanSocket();
		PostQuitMessage(0);
	}
	return 0;

	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
