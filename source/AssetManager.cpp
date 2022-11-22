#include "AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::vector<char> AssetManager::readText(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to load file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void* AssetManager::readPixels(const std::string& filename, int& width, int& height, int& size)
{
	// Read data from texture file
	int numChannels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);

	size = width * height * numChannels;

	if (!pixels)
	{
		throw std::runtime_error("failed to load file");
	}

	return pixels;
}

void AssetManager::releasePixels(void* pixels)
{
	stbi_image_free(pixels);
}