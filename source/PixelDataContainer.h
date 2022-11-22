#pragma once

#include "AssetManager.h"
#include <string>

/**
* @brief Support for texture reading.
*/
class PixelDataContainer
{

public:

	/**
	* @brief Creates a PixelDataContainer with texture data from a file.
	* 
	* @param filename location of texture data to read
	*/
	PixelDataContainer(const std::string& filename);

	~PixelDataContainer();

	/**
	* @brief Returns texture width.
	* 
	* @return width of texture in pixels
	*/
	int getWidth() const;

	/**
	* @brief Returns texture height.
	* 
	* @return height of texture in pixels
	*/
	int getHeight() const;

	/**
	* @brief Returns texture size in bytes.
	* 
	* @return number of bytes pixel data occupies
	*/
	int getSizeInBytes() const;

	/**
	* @brief Returns handle to pixel data.
	* 
	* @return pointer to pixel data
	*/
	void* getData() const;

private:
	void* pixels;
	int width;
	int height;
	int numBytes;
};