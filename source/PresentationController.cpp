#include "PresentationController.h"

PresentationController::PresentationController(VulkanCore& vulkanCoreSupport, Image& sampler) : vulkanCoreSupport(vulkanCoreSupport), swapChain(vulkanCoreSupport)
{
	// get images
	std::vector<VkImage> vulkanImages(swapChain.getNumImages());
	swapChain.getImages(vulkanImages.data());

	// create Image object wrappers

	// Make sure to reserve space so the vector won't allocate during the loop.
	// The problem is that, unfortunately, an Image cannot safely be destroyed
	// until it has been registered with the gpu API. Since that happens later,
	// we need to ensure the STL containers don't copy Image objects until they
	// are registered.
	images.reserve(swapChain.getNumImages()); 
	for (size_t i = 0; i < swapChain.getNumImages(); i++)
	{
		images.emplace_back(vulkanCoreSupport, vulkanImages.at(i), swapChain.getFormat(), vulkanCoreSupport.getWindowResolution());
	}

	//create a Pass object for each image
	passes.reserve(swapChain.getNumImages());
	for (size_t i = 0; i < swapChain.getNumImages(); i++)
	{
		passes.emplace_back(vulkanCoreSupport, sampler, images.at(i));
	}
			
	createSyncObjects();
}

PresentationController::~PresentationController()
{
	VkDevice device = vulkanCoreSupport.getDevice();

	for (size_t i = 0; i < swapChain.getNumImages(); i++)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
}

void PresentationController::present()
{
	VkDevice device = vulkanCoreSupport.getDevice();

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain.getSwapChainObject(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image");
	}

	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	// copy complete frame into swapchain image

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &passes[currentFrame].getCommandBuffer();
	
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	   

	if (vkQueueSubmit(vulkanCoreSupport.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer");
	}

	// submit to present queue

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain.getSwapChainObject() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(vulkanCoreSupport.getPresentQueue(), &presentInfo);

	currentFrame = (currentFrame + 1) % swapChain.getNumImages();
}

void PresentationController::createSyncObjects()
{
	imageAvailableSemaphores.resize(swapChain.getNumImages());
	renderFinishedSemaphores.resize(swapChain.getNumImages());
	inFlightFences.resize(swapChain.getNumImages());
	
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < swapChain.getNumImages(); i++)
	{
		if (vkCreateSemaphore(vulkanCoreSupport.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
			|| vkCreateSemaphore(vulkanCoreSupport.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
			|| vkCreateFence(vulkanCoreSupport.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects");
		}
	}
}

void PresentationController::getPasses(std::vector<Pass*>& output)
{
	output.clear();
	output.resize(passes.size());
	for (int i = 0; i < passes.size(); i++)
	{	
		output.at(i) = &passes.at(i);
	}
}