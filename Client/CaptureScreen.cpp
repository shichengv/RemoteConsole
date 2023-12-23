#include "framework.h"
#include "Client.h"

int CaptureScreen(TCHAR * PathName) {

	char* lpbitmap = NULL;

	BITMAP bmp_screen;
	DWORD dw_bmp_size = 0;
	DWORD dw_size_of_dib = 0;
	DWORD dw_bytes_written = 0;

	HANDLE h_dib = NULL;
	HANDLE hfile = NULL;

	// 标识显示相应输出的窗口来获取显示 DC
	HDC screen = GetDC(NULL);
	HDC window = GetDC(NULL);
	// 调用 CreateCompatibleDC 函数来创建内存 DC
	HDC memory = CreateCompatibleDC(screen);

	int width = GetDeviceCaps(screen, HORZRES);
	int height = GetDeviceCaps(screen, VERTRES);

	//	创建屏幕图像位图
	HBITMAP hbm_screen = CreateCompatibleBitmap(screen, width, height);

	// SelectObject 函数将对象选择到指定的设备上下文 (DC) 。 新的 对象替换相同类型的上一个对象。
	HBITMAP old_hbitmap = (HBITMAP)SelectObject(memory, hbm_screen);

	// 从显示dc传输到内存dc 
	// 将源矩形直接复制到目标矩形
	BitBlt(memory, 0, 0, width, height, screen, 0, 0, SRCCOPY);

	hbm_screen = (HBITMAP)SelectObject(memory, old_hbitmap);

	GetObject(hbm_screen, sizeof(BITMAP), &bmp_screen);

	BITMAPFILEHEADER   bmfHeader;
	BITMAPINFOHEADER   bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmp_screen.bmWidth;
	bi.biHeight = bmp_screen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	
	dw_bmp_size = ((bmp_screen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp_screen.bmHeight;

	h_dib = GlobalAlloc(GHND, dw_bmp_size);
	lpbitmap = (char*)GlobalLock(h_dib);


	GetDIBits(window, hbm_screen, 0,
		(UINT)bmp_screen.bmHeight,
		lpbitmap,
		(BITMAPINFO*)&bi, DIB_RGB_COLORS);
	
	hfile = CreateFile(PathName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	dw_size_of_dib = dw_bmp_size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	bmfHeader.bfSize = dw_size_of_dib;

	// bfType must always be BM for Bitmaps
	bmfHeader.bfType = 0x4D42;

	WriteFile(hfile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dw_bytes_written, NULL);
	WriteFile(hfile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dw_bytes_written, NULL);
	WriteFile(hfile, (LPSTR)lpbitmap, dw_bmp_size, &dw_bytes_written, NULL);

	GlobalUnlock(h_dib);
	GlobalFree(h_dib);

	CloseHandle(hfile);

	
	DeleteObject(hbm_screen);
	DeleteObject(memory);
	ReleaseDC(NULL, screen);
	ReleaseDC(NULL, window);

	return 0;
}
