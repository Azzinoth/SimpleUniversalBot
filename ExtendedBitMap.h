#pragma once
#include <Windows.h>

class ExtendedBitMap
{
private:
	BYTE* Data = nullptr;
	HBITMAP HBitMap = nullptr;
	int width = 0;
	int height = 0;
public:
	ExtendedBitMap(bool CaptureScreen = false)
	{
		if (CaptureScreen)
		{
			HWND hwnd;
			RECT rect;

			hwnd = GetDesktopWindow();
			GetClientRect(hwnd, &rect);

			// my windows 175 % scaling
			rect.right *= 1.75;
			this->width = rect.right;
			rect.bottom *= 1.75;
			this->height = rect.bottom;

			HDC hScreen = GetDC(GetDesktopWindow());
			HDC hdcMem = CreateCompatibleDC(hScreen);
			this->HBitMap = CreateCompatibleBitmap(hScreen, this->width, this->height);
			HGDIOBJ hOld = SelectObject(hdcMem, this->HBitMap);
			BitBlt(hdcMem, 0, 0, this->width, this->height, hScreen, 0, 0, SRCCOPY);
			SelectObject(hdcMem, hOld);

			BITMAPINFOHEADER bmi = { 0 };
			bmi.biSize = sizeof(BITMAPINFOHEADER);
			bmi.biPlanes = 1;
			bmi.biBitCount = 32;
			bmi.biWidth = this->width;
			bmi.biHeight = -this->height;
			bmi.biCompression = BI_RGB;
			bmi.biSizeImage = 0;

			if (this->Data) delete this->Data;
			this->Data = new BYTE[4 * this->width * this->height];

			GetDIBits(hdcMem, this->HBitMap, 0, this->height, this->Data, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

			ReleaseDC(GetDesktopWindow(), hScreen);
			DeleteDC(hdcMem);
		}
	}

	~ExtendedBitMap() 
	{
		delete this->Data;
		DeleteObject(this->HBitMap);
	}

	BYTE* getData()
	{
		return this->Data;
	}

	HBITMAP* getHBitmap()
	{
		return &this->HBitMap;
	}

	int getWidth()
	{
		return this->width;
	}

	int getHeight()
	{
		return this->height;
	}

	void draw(HDC hdc, int x, int y)
	{
		if (this->Data == nullptr) return;

		HDC hdcMem;
		HGDIOBJ oldBitmap;
		BITMAP bitmap;

		hdcMem = CreateCompatibleDC(hdc);
		oldBitmap = SelectObject(hdcMem, this->HBitMap);

		GetObject(this->HBitMap, sizeof(bitmap), &bitmap);

		BitBlt(hdc, x, y, this->width, this->height, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);
	}

	void captureScreen(int x = 0, int y = 0, int width = 0, int height = 0)
	{
		if (this->Data)
		{
			delete this->Data;
			this->Data = nullptr;
			DeleteObject(this->HBitMap);
		}

		if (width == 0 || height == 0)
		{
			HWND hwnd;
			RECT rect;

			hwnd = GetDesktopWindow();
			GetClientRect(hwnd, &rect);

			// my windows 175 % scaling
			rect.right *= 1.75;
			this->width = rect.right;
			rect.bottom *= 1.75;
			this->height = rect.bottom;
		}
		else
		{
			this->width = width;
			this->height = height;
		}

		HDC hScreen = GetDC(GetDesktopWindow());
		HDC hdcMem = CreateCompatibleDC(hScreen);
		this->HBitMap = CreateCompatibleBitmap(hScreen, this->width, this->height);
		HGDIOBJ hOld = SelectObject(hdcMem, this->HBitMap);
		BitBlt(hdcMem, 0, 0, this->width, this->height, hScreen, x, y, SRCCOPY);
		SelectObject(hdcMem, hOld);

		BITMAPINFOHEADER bmi = { 0 };
		bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmi.biPlanes = 1;
		bmi.biBitCount = 32;
		bmi.biWidth = this->width;
		bmi.biHeight = -this->height;
		bmi.biCompression = BI_RGB;
		bmi.biSizeImage = 0;

		if (this->Data) delete this->Data;
		this->Data = new BYTE[4 * this->width * this->height];

		GetDIBits(hdcMem, this->HBitMap, 0, this->height, this->Data, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

		ReleaseDC(GetDesktopWindow(), hScreen);
		DeleteDC(hdcMem);
	}

	int compareWith(ExtendedBitMap* otherImage)
	{
		if (otherImage->getData() == nullptr) return 0;
		if (otherImage == this) return 100;

		if (otherImage->getWidth() != this->getWidth() ||
			otherImage->getHeight() != this->getHeight()) return 0;

		int dataLength = 4 * this->getWidth() * this->getHeight();
		int count = 0;
		for (int i = 0; i < dataLength; i += 4)
		{
			if ((otherImage->getData()[i] == this->getData()[i]) &&
				(otherImage->getData()[i + 1] == this->getData()[i + 1]) &&
				(otherImage->getData()[i + 2] == this->getData()[i + 2]))
				count++;
		}

		return float(count) / float(dataLength / 4) * 100;
	}

	int compareWithRegion(ExtendedBitMap* otherImage, int beginX, int beginY)
	{
		if (otherImage->getData() == nullptr) return 0;
		if (otherImage == this) return 100;

		ExtendedBitMap* smallerImage = nullptr;
		ExtendedBitMap* biggerImage = nullptr;
		if (otherImage->getWidth() > this->getWidth() && otherImage->getHeight() > this->getHeight())
		{
			smallerImage = this;
			biggerImage = otherImage;
		}
		else if (otherImage->getWidth() < this->getWidth() && otherImage->getHeight() < this->getHeight())
		{
			smallerImage = otherImage;
			biggerImage = this;
		}
		else
		{
			return 0;
		}

		//4 * ((y * screenW) + x)
		int count = 0;
		int dataLength = 4 * smallerImage->getWidth() * smallerImage->getHeight();
		int biggerImageIndex = 0;
		int smallerImageIndex = 0;

		for (int i = beginX; i < beginX + smallerImage->getWidth(); i += 1)
		{
			for (int j = beginY; j < beginY + smallerImage->getHeight(); j += 1)
			{
				biggerImageIndex = 4 * ((j * biggerImage->getWidth()) + i);
				smallerImageIndex = 4 * (((j - beginY) * smallerImage->getWidth()) + (i - beginX));

				if ((biggerImage->getData()[biggerImageIndex] == smallerImage->getData()[smallerImageIndex]) &&
					(biggerImage->getData()[biggerImageIndex + 1] == smallerImage->getData()[smallerImageIndex + 1]) &&
					(biggerImage->getData()[biggerImageIndex + 2] == smallerImage->getData()[smallerImageIndex + 2]))
					count++;
			}
		}

		/*int dataLength = 4 * smallerImage->getWidth() * smallerImage->getHeight();
		int beginIndex = (beginX * biggerImage->getWidth() + beginY) * 4;
		int count = 0;
		for (int i = beginIndex; i < beginIndex + dataLength; i += 4)
		{
			if ((biggerImage->getData()[i] == smallerImage->getData()[i - beginIndex]) &&
				(biggerImage->getData()[i + 1] == smallerImage->getData()[i + 1 - beginIndex]) &&
				(biggerImage->getData()[i + 2] == smallerImage->getData()[i + 2 - beginIndex]))
				count++;
		}*/

		return float(count) / float(dataLength / 4) * 100;
	}
};