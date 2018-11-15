#pragma once

class ImageDataClass
{
private :
	BYTE* Data = nullptr;
	int colourDepth = 4;
	int width = 0;
	int height = 0;
public	:
	ImageDataClass(int colourDepth, int width, int height)
	{
		Data = new BYTE[colourDepth * width * height];
		this->colourDepth = colourDepth;
		this->width = width;
		this->height = height;
	}

	BYTE* getData()
	{
		return Data;
	}

	int getColourDepth()
	{
		return this->colourDepth;
	}

	int getWidth()
	{
		return this->width;
	}

	int getHeight()
	{
		return this->height;
	}

	int compareWith(ImageDataClass* otherImageData)
	{
		if (otherImageData->getData() == nullptr) return 0;
		if (otherImageData->getData() == this->getData()) return 100;

		if (otherImageData->getWidth() != this->getWidth() ||
			otherImageData->getHeight() != this->getHeight()) return 0;

		int dataLength = this->getColourDepth() * this->getWidth() * this->getHeight();
		int count = 0;
		for (int i = 0; i < dataLength; i+=4)
		{
			if ((otherImageData->getData()[i] == this->getData()[i]) &&
			    (otherImageData->getData()[i + 1] == this->getData()[i + 1]) &&
				(otherImageData->getData()[i + 2] == this->getData()[i + 2]))
			count++;
		}

		return float(count) / float(dataLength / 4) * 100;
	}

	~ImageDataClass()
	{
		delete Data;
	}
};