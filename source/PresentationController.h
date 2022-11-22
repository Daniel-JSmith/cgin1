#pragma once

#include "PresentPass.h"
#include "SwapChain.h"

/**
* @brief Controller managing presentation.
*/
class PresentationController
{
public:

	/**
	* @brief Creates a PresentationController.
	* 
	* @param sampler image to display
	*/
	PresentationController(VulkanCore& vulkanCoreSupport, Image& sampler);

	~PresentationController();

	/**
	* @brief Submits presentation commands to GPU.
	* 
	*/
	void present();

	/**
	* @brief Returns passes used by this PresentationController.
	* 
	* @param output variable to store passes
	*/
	void getPasses(std::vector<Pass*>& output);

private:

	VulkanCore& vulkanCoreSupport;

	SwapChain swapChain;
	std::vector<Image> images;
	std::vector<PresentPass> passes;

	void createSyncObjects();
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
};
