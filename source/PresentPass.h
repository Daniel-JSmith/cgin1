#pragma once

#include "Pass.h"

/**
* @brief Pass that prepares an Image for presentation.
* 
* Only used internally; users shouldn't need to instantiate a PresentPass.
*/
class PresentPass : public Pass
{
public:
	/**
	* @brief Creates a PresentPass.
	* 
	* @param sourceImage image to blit from
	* @param swapChainImage image that is part of swap chain
	*/
	PresentPass(VulkanCore& vulkanCoreSupport, Image& sourceImage, Image& swapChainImage);
	~PresentPass();

	void prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers) override;

private:
	Image& sourceImage;
	Image& swapChainImage;

	void recordCommandBuffer(std::function<void(VkCommandBuffer, Pass*)> insertBarriers) override;
};