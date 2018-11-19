#pragma once

class FindImageInRegion
{
private:
	ExtendedBitMap* image = nullptr;
	int startX = -1;
	//int startY = -1;
public:
	FindImageInRegion(int beginX, int beginY, int width, int height)
	{
		this->image = new ExtendedBitMap(false);
		this->image -> captureScreen(beginX, beginY, width, height);

		this->startX = beginX;
	}

	~FindImageInRegion()
	{
		if (this->image) delete this->image;
	}

	ExtendedBitMap*& getImage()
	{
		return this->image;
	}

	int getStartX()
	{
		return this->startX;
	}
};