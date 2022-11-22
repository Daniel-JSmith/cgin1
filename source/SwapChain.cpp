#include "SwapChain.h"
#include <algorithm>

void querySwapChainSupport(VkSurfaceCapabilitiesKHR& capabilities, std::vector<VkSurfaceFormatKHR>& formats, std::vector<VkPresentModeKHR>& presentModes, VulkanCore& vulkanCoreSupport)
{
	// capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanCoreSupport.getPhysicalDevice(), vulkanCoreSupport.getSurface(), &capabilities);

	// formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanCoreSupport.getPhysicalDevice(), vulkanCoreSupport.getSurface(), &formatCount, nullptr);
	if (formatCount != 0)
	{
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanCoreSupport.getPhysicalDevice(), vulkanCoreSupport.getSurface(), &formatCount, formats.data());
	}

	// present modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanCoreSupport.getPhysicalDevice(), vulkanCoreSupport.getSurface(), &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanCoreSupport.getPhysicalDevice(), vulkanCoreSupport.getSurface(), &presentModeCount, presentModes.data());
	}
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

SwapChain::SwapChain(VulkanCore& vulkanCoreSupport) : vulkanCoreSupport(vulkanCoreSupport)
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
	querySwapChainSupport(capabilities, formats, presentModes, vulkanCoreSupport);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);

	numImages = std::max(3u, capabilities.minImageCount);
	numImages = std::min(numImages, capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = vulkanCoreSupport.getSurface();

	createInfo.minImageCount = numImages;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = vulkanCoreSupport.getWindowResolution();
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	if (vulkanCoreSupport.getGraphicsQueueFamilyIndex() != vulkanCoreSupport.getPresentQueueFamilyIndex())
	{
		uint32_t indices[] = { vulkanCoreSupport.getGraphicsQueueFamilyIndex(), vulkanCoreSupport.getPresentQueueFamilyIndex() };

		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = indices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(vulkanCoreSupport.getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain");
	}

	//
	vkGetSwapchainImagesKHR(vulkanCoreSupport.getDevice(), swapChain, &numImages, nullptr);
	imageFormat = surfaceFormat.format;
}

SwapChain::~SwapChain()
{
	vkDestroySwapchainKHR(vulkanCoreSupport.getDevice(), swapChain, nullptr);
}

VkSwapchainKHR SwapChain::getSwapChainObject() const
{
	return swapChain;
}

uint32_t SwapChain::getNumImages() const
{
	return numImages;
}

void SwapChain::getImages(VkImage* images)
{
	vkGetSwapchainImagesKHR(vulkanCoreSupport.getDevice(), swapChain, &numImages, images);
}

VkFormat SwapChain::getFormat() const
{
	return imageFormat;	
}