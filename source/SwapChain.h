#pragma once

#include "VulkanCore.h"

/**
* @brief Wrapper over GPU swapchain objects.
*/
class SwapChain
{

public:

	SwapChain(VulkanCore& vulkanCoreSupport);

	~SwapChain();

	/**
	* @brief Returns underlying Vulkan swap chain object.
	* 
	* @return handle to Vulkan swap chain object
	*/
	VkSwapchainKHR getSwapChainObject() const;

	/**
	* @brief Returns the number of images in the swap chain.
	* 
	* @return number of images in the swap chain
	*/
	uint32_t getNumImages() const;

	/**
	* @brief Returns the Images of this SwapChain.
	* 
	* @param images variable to store returned images
	*/
	void getImages(VkImage* images);

	/**
	* @brief Returns the format of the swap chain images.
	* 
	* @return swap chain image format
	*/
	VkFormat getFormat() const;

private:

	VulkanCore& vulkanCoreSupport;

	VkSwapchainKHR swapChain;
	uint32_t numImages;
	VkFormat imageFormat;
};