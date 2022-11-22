#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <vector>
#include <fstream>

#include "VulkanCore.h"

/**
* @brief Container for file I/O functions.
* Provides functions to read shader text and texture data.
*/
class AssetManager
{

public:

	/**
	* @brief Opens filename and returns its contents interpreted as text.
	* 
	* @param filename location of file to read
	* 
	* @return file content
	*/
	static std::vector<char> readText(const std::string& filename);

	/**
	* @brief Opens filename and returns its contents interpreted as an image.
	* 
	* @param filename location of file to read
	* @param width width of image read in pixels
	* @param height height of image read in pixels
	* @param size size of image read in bytes
	* 
	* @return pointer to image data.
	*/
	static void* readPixels(const std::string& filename, int& width, int& height, int& size);

	/**
	* @brief Frees a read image.
	* 
	* @param pixels pointer to image data
	*/
	static void releasePixels(void* pixels);
};