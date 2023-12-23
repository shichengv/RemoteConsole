#include "framework.h"
#include "Console.h"

extern HFONT hFont;
extern HWND hwndMainWnd;
extern HMENU hMenuListBoxItem;	// Pop-up menu handle

extern int cxChar, cxCaps, cyChar;
extern int iMaxWidth;

static int cxChild, cyChild;

TCHAR DebugConsoleBuffer[OUTPUTSIZE];

LRESULT CALLBACK ListWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	switch (message)
	{

	case WM_LBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		int index = SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(x, y));
		if (index != LB_ERR && HIWORD(index) == 0)
		{
			SendMessage(hwnd, LB_SETCURSEL, LOWORD(index), 0);

		}
	}
	break;

	case WM_RBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		ClientToScreen(hwnd, (LPPOINT)&lParam);
		TrackPopupMenu(hMenuListBoxItem, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, hwndMainWnd, NULL);

	}
	break;

	}

	return DefSubclassProc(hwnd, message, wParam, lParam);

}
extern LRESULT CALLBACK DebugConsoleWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {

	switch (message)
	{
	//case WM_USER: // user-defined message
	//	// Update the text of the child window using SetWindowText
	//	SetWindowText(hwnd, (LPCWSTR)lParam);
	//	break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;

}

LRESULT CALLBACK ConsoleWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){

	int iVertPos, iHorzPos, iPaintBeg, iPaintEnd;
	int i, x, y;

	TCHAR szBuffer[10];

	PAINTSTRUCT ps;
	HDC hdc;
	SCROLLINFO si;

	switch (message)
	{

	case WM_CREATE:
	{

	}
	return 0;

	case WM_SIZE:
	{
		cxChild = LOWORD(lParam);
		cyChild = HIWORD(lParam);

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = NUMLINES - 1;
		si.nPage = cyChild / cyChar;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 2 + iMaxWidth / cxChar;
		si.nPage = cxChild / cxChar;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

	}
	return 0;

	case WM_USER:
	{

	}
	break;

	case WM_VSCROLL:
	{
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);

		iVertPos = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_TOP:
			si.nPos = si.nMin;
			break;
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;
		case SB_LINEUP:
			si.nPos -= 1;
			break;
		case SB_LINEDOWN:
			si.nPos += 1;
			break;
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}

		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_VERT, &si);
		if (si.nPos != iVertPos)
		{
			ScrollWindow(hwnd, 0, cyChar * (iVertPos - si.nPos), NULL, NULL);
			UpdateWindow(hwnd);
		}
	}
	return 0;

	case WM_HSCROLL:
	{
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;

		// Save the position for comparison later on

		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;

		switch (LOWORD(wParam))
		{
		case SB_LINELEFT:
			si.nPos -= 1;
			break;

		case SB_LINERIGHT:
			si.nPos += 1;
			break;

		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;

		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;

		case SB_THUMBPOSITION:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}
		// Set the position and then retrieve it.  Due to adjustments
		//   by Windows it may not be the same as the value set.

		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hwnd, SB_HORZ, &si);

		// If the position has changed, scroll the window 

		if (si.nPos != iHorzPos)
		{
			ScrollWindow(hwnd, cxChar * (iHorzPos - si.nPos), 0,
				NULL, NULL);
		}
	}
	return 0;

	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		if (hFont)
			SelectObject(hdc, hFont);

		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVertPos = si.nPos;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;

		iPaintBeg = max(0, iVertPos + ps.rcPaint.top / cyChar);
		iPaintEnd = min(NUMLINES - 1,
			iVertPos + ps.rcPaint.bottom / cyChar);

		for (i = iPaintBeg; i <= iPaintEnd; i++)
		{
			x = cxChar * (1 - iHorzPos);
			y = cyChar * (i - iVertPos);

			TextOut(hdc, x, y,
				sysmetrics[i].szLabel, lstrlen(sysmetrics[i].szLabel));
			TextOut(hdc, x + 22 * cxCaps, y,
				sysmetrics[i].szDesc, lstrlen(sysmetrics[i].szDesc));
			SetTextAlign(hdc, TA_RIGHT | TA_TOP);
			TextOut(hdc, x + 22 * cxCaps + 40 * cxChar, y, szBuffer,
				wsprintf(szBuffer, TEXT("%5d"),
					GetSystemMetrics(sysmetrics[i].iIndex)));
			SetTextAlign(hdc, TA_LEFT | TA_TOP);

		}

		EndPaint(hwnd, &ps);
	}
	return 0;

	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	break;

	default:
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}