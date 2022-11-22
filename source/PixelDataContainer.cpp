#include "PixelDataContainer.h"

PixelDataContainer::PixelDataContainer(const std::string& filename)
{
	pixels = AssetManager::readPixels(filename, width, height, numBytes);
}

PixelDataContainer::~PixelDataContainer()
{
	AssetManager::releasePixels(pixels);
}

int PixelDataContainer::getWidth() const
{
	return width;
}

int PixelDataContainer::getHeight() const
{
	return height;
}

int PixelDataContainer::getSizeInBytes() const
{
	return numBytes;
}

void* PixelDataContainer::getData() const
{
	return pixels;
}