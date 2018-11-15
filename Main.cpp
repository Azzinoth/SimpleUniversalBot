#include <iostream>
#include <Windows.h>
#include <windowsx.h>
//#include <Windows.h>
#include <cassert> // assert
#include "ImageDataClass.h"

HWND main_hwnd;
HDC g_hDC;
static TCHAR szWindowClass[] = TEXT("win32app");
static TCHAR szTitle[] = TEXT("SimpleUniversalBot");
HINSTANCE hInst;

HBITMAP tempImgToDraw = NULL;

/* forward declarations */
PBITMAPINFO CreateBitmapInfoStruct(HBITMAP);
void CreateBMPFile(LPTSTR pszFile, HBITMAP hBMP);

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD    cClrBits;

	// Retrieve the bitmap color format, width, and height.  
	assert(GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp));

	// Convert the color format to a count of bits.  
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	// Allocate memory for the BITMAPINFO structure. (This structure  
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
	// data structures.)  

	if (cClrBits < 24)
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * (1 << cClrBits));

	// There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

	else
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER));

	// Initialize the fields in the BITMAPINFO structure.  

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

	// If the bitmap is not compressed, set the BI_RGB flag.  
	pbmi->bmiHeader.biCompression = BI_RGB;

	// Compute the number of bytes in the array of color  
	// indices and store the result in biSizeImage.  
	// The width must be DWORD aligned unless the bitmap is RLE 
	// compressed. 
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
		* pbmi->bmiHeader.biHeight;
	// Set biClrImportant to 0, indicating that all of the  
	// device colors are important.  
	pbmi->bmiHeader.biClrImportant = 0;
	return pbmi;
}

void CreateBMPFile(LPTSTR pszFile, HBITMAP hBMP)
{
	HANDLE hf;                 // file handle  
	BITMAPFILEHEADER hdr;       // bitmap file-header  
	PBITMAPINFOHEADER pbih;     // bitmap info-header  
	LPBYTE lpBits;              // memory pointer  
	DWORD dwTotal;              // total count of bytes  
	DWORD cb;                   // incremental count of bytes  
	BYTE *hp;                   // byte pointer  
	DWORD dwTmp;
	PBITMAPINFO pbi;
	HDC hDC;

	hDC = CreateCompatibleDC(GetWindowDC(GetDesktopWindow()));
	SelectObject(hDC, hBMP);

	pbi = CreateBitmapInfoStruct(hBMP);

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	assert(lpBits);

	// Retrieve the color table (RGBQUAD array) and the bits  
	// (array of palette indices) from the DIB.  
	assert(GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
		DIB_RGB_COLORS));

	// Create the .BMP file.  
	hf = CreateFile(pszFile,
		GENERIC_READ | GENERIC_WRITE,
		(DWORD)0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	assert(hf != INVALID_HANDLE_VALUE);

	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
								// Compute the size of the entire file.  
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	// Compute the offset to the array of color indices.  
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD);

	// Copy the BITMAPFILEHEADER into the .BMP file.  
	assert(WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
		(LPDWORD)&dwTmp, NULL));

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
	assert(WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
		+ pbih->biClrUsed * sizeof(RGBQUAD),
		(LPDWORD)&dwTmp, (NULL)));

	// Copy the array of color indices into the .BMP file.  
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	assert(WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL));

	// Close the .BMP file.  
	assert(CloseHandle(hf));

	// Free memory.  
	GlobalFree((HGLOBAL)lpBits);
}

// Global variables of screen resolution
int screenW = 0;
int screenH = 0;

void InitScreenResolutionInfo()
{
	HWND hwnd;
	RECT rect;

	hwnd = GetDesktopWindow();
	GetClientRect(hwnd, &rect);

	// my windows 175 % scaling
	rect.right *= 1.75;
	screenW = rect.right;
	rect.bottom *= 1.75;
	screenH = rect.bottom;
}

int XcoordToNormalize(int x)
{
	return 65535 * (float(x) / float(screenW));
}

int YcoordToNormalize(int y)
{
	return 65535 * (float(y) / float(screenH));
}

void MouseMoveTo(int x, int y)
{
	// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/ns-winuser-tagmouseinput
	// Remarks

	int tempX = 65535 * (float(x) / float(screenW));
	int tempY = 65535 * (float(y) / float(screenH));

	INPUT Inputs[1] = { 0 };

	Inputs[0].type = INPUT_MOUSE;
	Inputs[0].mi.dx = tempX; // desired X coordinate
	Inputs[0].mi.dy = tempY; // desired Y coordinate
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

	SendInput(1, Inputs, sizeof(INPUT));
}

void MouseClick(int x, int y)
{
	// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/ns-winuser-tagmouseinput
	// Remarks

	INPUT Inputs[3] = { 0 };

	Inputs[0].type = INPUT_MOUSE;
	Inputs[0].mi.dx = XcoordToNormalize(x); // desired X coordinate
	Inputs[0].mi.dy = YcoordToNormalize(y); // desired Y coordinate
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

	Inputs[1].type = INPUT_MOUSE;
	Inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	Inputs[2].type = INPUT_MOUSE;
	Inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(3, Inputs, sizeof(INPUT));
}

void MouseDrag(int x, int y, int x1, int x2)
{
	// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/ns-winuser-tagmouseinput
	// Remarks

	INPUT Inputs[4] = { 0 };

	Inputs[0].type = INPUT_MOUSE;
	Inputs[0].mi.dx = XcoordToNormalize(x); // desired X coordinate
	Inputs[0].mi.dy = YcoordToNormalize(y); // desired Y coordinate
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE; 

	Inputs[1].type = INPUT_MOUSE;
	Inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	Inputs[2].type = INPUT_MOUSE;
	Inputs[2].mi.dx = XcoordToNormalize(x1); // desired X coordinate
	Inputs[2].mi.dy = YcoordToNormalize(x2); // desired Y coordinate
	Inputs[2].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

	Inputs[3].type = INPUT_MOUSE;
	Inputs[3].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(4, Inputs, sizeof(INPUT));
}

void MouseDragt(int x, int y, int x2, int y2)
{
	// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/ns-winuser-tagmouseinput
	// Remarks

	//int tempX = 65535 * (float(x) / float(screenW));
	//int tempY = 65535 * (float(y) / float(screenH));

	//int tempXE = 65535 * (float(x1) / float(screenW));
	//int tempYE = 65535 * (float(x2) / float(screenH));

	//SetCursorPos(x / 1.75, y / 1.75);
	//mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); //clicks on canvas
	//Sleep(30);                                      //puts small delay between next click
	//mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);  // holds down left mouse button
	//Sleep(300);
	//SetCursorPos(x1 / 1.75, x2 / 1.75);
	//mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);  //releases left mouse button

	//return;

	INPUT Inputs[1] = { 0 };
	INPUT Inputs1[1] = { 0 };
	INPUT Inputs2[1] = { 0 };
	INPUT Inputs3[1] = { 0 };

	Inputs[0].type = INPUT_MOUSE;
	Inputs[0].mi.dx = XcoordToNormalize(x); // desired X coordinate
	Inputs[0].mi.dy = YcoordToNormalize(y); // desired Y coordinate
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

	SendInput(1, Inputs, sizeof(INPUT));
	Sleep(350);

	Inputs1[0].type = INPUT_MOUSE;
	Inputs1[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	SendInput(1, Inputs1, sizeof(INPUT));
	Sleep(350);

	int difference = YcoordToNormalize(y2) - YcoordToNormalize(y);

	for (int i = 1; i <= 10; i++)
	{
		Inputs1[0].type = INPUT_MOUSE;
		Inputs1[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		Sleep(20);

		Inputs2[0].type = INPUT_MOUSE;
		Inputs2[0].mi.dx = XcoordToNormalize(x2);
		//Inputs2[0].mi.dy = YcoordToNormalize(y2) - i * 200;
		int currentDiff = ((float(i) * 10.0 / 100.0) * difference);
		Inputs2[0].mi.dy = YcoordToNormalize(y) + currentDiff;
		Inputs2[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

		SendInput(1, Inputs2, sizeof(INPUT));
		Sleep(20);
		ZeroMemory(&Inputs2, sizeof(INPUT));
	}

	Sleep(550);
	Inputs3[0].type = INPUT_MOUSE;
	Inputs3[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(1, Inputs3, sizeof(INPUT));
}

void MouseWheel(int amountOfMovementUp)
{
	// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/ns-winuser-tagmouseinput
	// Remarks

	INPUT Inputs[1] = { 0 };

	Inputs[0].type = INPUT_MOUSE;
	Inputs[0].mi.mouseData = amountOfMovementUp; // desired Y coordinate
	Inputs[0].mi.dwFlags = MOUSEEVENTF_WHEEL;

	SendInput(1, Inputs, sizeof(INPUT));
}

void ScrollBack()
{
	for (int i = 0; i < 15; i++)
	{
		MouseDragt(2030, 900, 2030, 1500);
		Sleep(150);
	}
}

//BYTE* ScreenData = 0;
ImageDataClass* ScreenData;

void GetScreenDataRegion(ImageDataClass** data, int beginX, int beginY, int width, int length)
{
	HDC hScreen = GetDC(GetDesktopWindow());

	HDC hdcMem = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, screenW, screenH);
	HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
	BitBlt(hdcMem, 0, 0, width, length, hScreen, beginX, beginY, SRCCOPY);
	SelectObject(hdcMem, hOld);

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = width;
	bmi.biHeight = -length;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;

	*data = new ImageDataClass(4, width, length);

	GetDIBits(hdcMem, hBitmap, 0, length, (*data)->getData(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	ReleaseDC(GetDesktopWindow(), hScreen);
	DeleteDC(hdcMem);
	DeleteObject(hBitmap);
}

void ScreenCap()
{
	HDC hScreen = GetDC(GetDesktopWindow());

	HDC hdcMem = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, screenW, screenH);
	HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
	BitBlt(hdcMem, 0, 0, screenW, screenW, hScreen, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOld);

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = screenW;
	bmi.biHeight = -screenH;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;

	if (ScreenData)
		free(ScreenData);
	ScreenData = new ImageDataClass(4, screenW, screenH);//(BYTE*)malloc(4 * screenW * screenH);

	GetDIBits(hdcMem, hBitmap, 0, screenH, ScreenData->getData(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	ReleaseDC(GetDesktopWindow(), hScreen);
	DeleteDC(hdcMem);
	DeleteObject(hBitmap);
}

inline int PosB(int x, int y)
{
	return ScreenData->getData()[4 * ((y * screenW) + x)];
}

inline int PosG(int x, int y)
{
	return ScreenData->getData()[4 * ((y * screenW) + x) + 1];
}

inline int PosR(int x, int y)
{
	return ScreenData->getData()[4 * ((y * screenW) + x) + 2];
}

bool ButtonPress(int Key)
{
	bool button_pressed = false;

	while (GetAsyncKeyState(Key))
		button_pressed = true;

	return button_pressed;
}

//int main()
//{
//	InitScreenResolutionInfo();
//
//	//ScrollBack();
//
//	bool first = true;
//	ImageDataClass* IMDfirst = nullptr;
//
//	while (true)
//	{
//		if (ButtonPress(VK_SPACE))
//		{
//			// Print out current cursor position
//			POINT p;
//			GetCursorPos(&p);
//			p.x *= 1.75;
//			p.y *= 1.75;
//			printf("X:%d Y:%d \n", p.x, p.y);
//			// Print out RGB value at that position
//			//std::cout << "Bitmap: r: " << PosR(p.x, p.y) << " g: " << PosG(p.x, p.y) << " b: " << PosB(p.x, p.y) << "\n";
//			if (first)
//			{
//				GetScreenDataRegion(&IMDfirst, p.x, p.y, 10, 10);
//				first = false;
//			}
//			else
//			{
//				ImageDataClass* temp = nullptr;
//				GetScreenDataRegion(&temp, p.x, p.y, 10, 10);
//
//				std::cout << IMDfirst->compareWith(temp) << "\n";
//				//std::cout << " r: " << (int)temp->getData()[2]
//				//	<< " g: " << (int)temp->getData()[1]
//				//	<< " b: " << (int)temp->getData()[0]
//				//	<< "\n";
//
//				temp->~ImageDataClass();
//			}
//			
//		}
//		else if (ButtonPress(VK_ESCAPE))
//		{
//			printf("Quit\n");
//			break;
//		}
//		else if (ButtonPress(VK_SHIFT))
//		{
//			ScreenCap();
//			printf("Captured\n");
//		}
//	}
//
//	int count = 200;
//	while (true)
//	{
//		//MouseMoveTo(2460, 1300);
//		//MouseWheel(-250);
//
//		for (int i = 0; i < 20; i++)
//		{
//			MouseClick(2200, 900);
//			Sleep(10);
//			for (int j = 0; j < 3; j++)
//			{
//				MouseClick(2000, 900);
//			}
//			Sleep(150);
//			///
//			MouseClick(2200, 1200);
//			Sleep(10);
//			for (int j = 0; j < 3; j++)
//			{
//				MouseClick(2000, 1200);
//			}
//			Sleep(150);
//			///
//			MouseClick(2200, 1500);
//			Sleep(10);
//			for (int j = 0; j < 3; j++)
//			{
//				MouseClick(2000, 1500);
//			}
//			Sleep(150);
//			///
//
//			MouseDragt(2030, 1500, 2030, 1170);
//			Sleep(250);
//		}
//
//		ScrollBack();
//
//		return 0;
//	}
//
//	for (int i = 0; i < 500; i++)
//	{
//		MouseClick(3600, 1400);
//	}
//
//	return 0;
//
//	HWND hwnd;
//	HDC hdc[2];
//	HBITMAP hbitmap;
//
//	hwnd = GetDesktopWindow();
//	hdc[0] = GetWindowDC(hwnd);
//
//	hbitmap = CreateCompatibleBitmap(hdc[0], screenW, screenH);
//	hdc[1] = CreateCompatibleDC(hdc[0]);
//	SelectObject(hdc[1], hbitmap);
//
//	BitBlt(
//		hdc[1],
//		0,
//		0,
//		screenW,
//		screenH,
//		hdc[0],
//		0,
//		0,
//		SRCCOPY
//	);
//
//	CreateBMPFile(TEXT("D:\\bitmap.bmp"), hbitmap);
//
//	// get the device context of the screen
//	HDC hScreenDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
//	// and a device context to put it in
//	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
//
//	int width = GetSystemMetrics(SM_CXSCREEN);
//	int height = GetSystemMetrics(SM_CYSCREEN);
//
//	// maybe worth checking these are positive values
//	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
//
//	// get a new bitmap
//	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
//
//	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
//	hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);
//
//	// clean up
//	DeleteDC(hMemoryDC);
//	DeleteDC(hScreenDC);
//
//	// now your image is held in hBitmap. You can save it or do whatever with it
//
//	int temp = 0;
//	std::cout << width << std::endl;
//	std::cout << "Hello" << width + " x " + height << std::endl;
//	std::cin >> temp;
//}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, TEXT("Call to RegisterClassEx failed!"), TEXT("Win32 Guided Tour"), NULL);
		return 1;
	}

	hInst = hInstance;

	main_hwnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1280, 720,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!main_hwnd)
	{
		MessageBox(NULL, TEXT("Call to CreateWindow failed!"), TEXT("Win32 Guided Tour"), NULL);
		return 1;
	}
	else
	{
		RECT rc;
		GetClientRect(main_hwnd, &rc);

		InitScreenResolutionInfo();

		/*HWND hwnd;
		HDC hdc[2];

		hwnd = GetDesktopWindow();
		hdc[0] = GetWindowDC(hwnd);

		tempImgToDraw = CreateCompatibleBitmap(hdc[0], screenW, screenH);
		hdc[1] = CreateCompatibleDC(hdc[0]);
		SelectObject(hdc[1], tempImgToDraw);

		BitBlt(
			hdc[1],
			0,
			0,
			screenW,
			screenH,
			hdc[0],
			0,
			0,
			SRCCOPY
		);*/

		tempImgToDraw = (HBITMAP)LoadImage(hInst, L"D:\\1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		InvalidateRect(main_hwnd, 0, TRUE);
	}

	ShowWindow(main_hwnd, nCmdShow);
	UpdateWindow(main_hwnd);

	// Main message loop:
	MSG msg;

	ZeroMemory(&msg, sizeof(MSG));
	ShowCursor(true);

	RECT rc;
	GetClientRect(main_hwnd, &rc);

	//rc.right - rc.left, rc.bottom - rc.top

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			
			//
		}
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		PAINTSTRUCT     ps;
		HDC             hdc;
		BITMAP          bitmap;
		HDC             hdcMem;
		HGDIOBJ         oldBitmap;

		hdc = BeginPaint(hWnd, &ps);

		//if (tempImgToDraw != NULL)
		//{
			hdcMem = CreateCompatibleDC(hdc);
			oldBitmap = SelectObject(hdcMem, tempImgToDraw);

			GetObject(tempImgToDraw, sizeof(bitmap), &bitmap);
			
			BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);

			MoveToEx(hdc, 0, 0, NULL);
			LineTo(hdc, 50, 50);
		//}

		EndPaint(hWnd, &ps);
		//InvalidateRect(main_hwnd, 0, TRUE);
		
		break;

	case WM_KEYDOWN:
	{
		if (wParam == 0x1B/*VK_ESCAPE*/) PostQuitMessage(0);

		if (wParam == 0x26/*VK_UP*/)
		{
			//camera->setKey(1);
			//player->setKey(1);
		}

		if (wParam == 0x28/*VK_DOWN*/)
		{
			//camera->setKey(2);
			//player->setKey(2);
		}

		if (wParam == 37/*VK_LEFT*/)
		{
			//camera->setKey(3);
			//player->setKey(3);
		}

		if (wParam == 39/*VK_RIGHT*/)
		{
			//camera->setKey(4);
			//player->setKey(4);
		}

		if (wParam == VK_SPACE) {
			/*HWND hwnd;
			HDC hdc[2];
			
			hwnd = GetDesktopWindow();
			hdc[0] = GetWindowDC(hwnd);
			
			tempImgToDraw = CreateCompatibleBitmap(hdc[0], screenW, screenH);
			hdc[1] = CreateCompatibleDC(hdc[0]);
			SelectObject(hdc[1], tempImgToDraw);
			
			BitBlt(
				hdc[1],
				0,
				0,
				screenW,
				screenH,
				hdc[0],
				0,
				0,
				SRCCOPY
			);

			InvalidateRect(main_hwnd, 0, TRUE);*/
		}
	}
	break;

	case WM_KEYUP:
	{
		//camera->setKey(0);
		//player->setKey(0);
	}
	break;

	case WM_MOUSEMOVE:
	{
		if (hWnd == GetActiveWindow()) {
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);
			//POINT mouse;
			//GetCursorPos(&mouse);

			RECT rc, screen_wr;
			GetClientRect(hWnd, &rc);
			GetWindowRect(hWnd, &screen_wr);
		}
	}
	break;

	case WM_MOUSEWHEEL:
	{
		//camera->setMouseWheel(GET_Y_LPARAM(wParam));
	}
	break;

	case WM_LBUTTONUP:
	{
		//camera->setMouseLeftButtonState(false);
	}
	break;

	case WM_LBUTTONDOWN:
	{
		//camera->setMouseLeftButtonState(true);
	}
	break;

	case WM_RBUTTONUP:
	{
		//camera->setMouseRightButtonState(false);
	}
	break;

	case WM_RBUTTONDOWN:
	{
		//camera->setMouseRightButtonState(true);
	}
	break;

	return DefWindowProc(hWnd, message, wParam, lParam);

	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	}

	return 0;
};