#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>
#include <optional>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <functional>

#include "AccessSpecifier.h"

/**
* @brief Wrapper over low-level Vulkan API calls.
*/
class VulkanCore
{

public:

	VulkanCore(const VkExtent2D & renderResolution, const VkExtent2D & presentResolution, const std::string & windowName, const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions);

	~VulkanCore();

	/**
	* @return logical device handle
	*/
	VkDevice& getDevice();

	/**
	* @return physical devive handle
	*/
	VkPhysicalDevice getPhysicalDevice();

	/**
	* @return surface object handle
	*/
	VkSurfaceKHR getSurface();

	/**
	* @return index of graphics queue family
	*/
	uint32_t getGraphicsQueueFamilyIndex();

	/**
	* @return index of present queue family
	*/
	uint32_t getPresentQueueFamilyIndex();

	/**
	* @return graphics queue object
	*/
	VkQueue getGraphicsQueue();

	/**
	* @return present queue object
	*/
	VkQueue getPresentQueue();

	/**
	* @return whether the engine is currently running
	*/
	bool engineRunning();

	/**
	* @return window dimensions
	*/
	VkExtent2D getWindowResolution();

	/**
	* @return descriptor pool object
	*/
	VkDescriptorPool getDescriptorPool();

	/**
	* @return vma allocator object
	*/
	VmaAllocator getVmaAllocator();

	/**
	* @brief Creates a descriptor pool.
	*
	* @param descriptorsUsed descriptors that will be used while the engine runs
	* @param numPasses number of passes that will execute while the engine runs
	*/
	void createDescriptorPool(std::vector<VkDescriptorType> descriptorsUsed, int numPasses);

	// Contains only operations that require descriptors. Other operations are not here
	/**
	* @brief maps resource operations to the corresponding Vulkan enum
	*/
	static std::unordered_map<AccessSpecifier::OPERATION, VkDescriptorType> descriptorTypes;

	/**
	* @brief Submits a command buffer for GPU execution.
	*
	* @param commandBuffer commands to submit
	* @param signalFence fence to signal once commands complete
	*/
	void submitCommandBuffer(VkCommandBuffer& commandBuffer, VkFence signalFence);

	/**
	* @brief Returns underlying Vulkan command pool object.
	*
	* @return handle to Vulkan command pool object
	*/
	VkCommandPool getCommandPool();

	/**
	* @brief Executes commands and blocks until they finish.
	*
	* @param commands commands to send to GPU
	*/
	void executeInstantCommands(std::function<void(VkCommandBuffer)> commands);

	const VkExtent2D& getRenderResolution() const;

private:

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}


	void pickPhysicalDevice();
	bool checkValidationLayerSupport();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();

	void initVmaAllocator();

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	std::vector<const char*> getRequiredExtensions();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance
		, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance
		, VkDebugUtilsMessengerEXT debugMessenger
		, const VkAllocationCallbacks* pAllocator);

	void createInstance();

	void createSurface();
	void initWindow();

	VkInstance vInstance;

	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	GLFWwindow* window;

	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	VmaAllocator vmaAllocator;

	VkDescriptorPool descriptorPool;

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkDebugUtilsMessengerEXT debugMessenger;

	VkCommandPool commandPool;

	VkCommandBuffer instantBuffer;
	VkFence instantBufferReady;

	void createCommandPool();
	void setupInstantCommands();

	/**
	* @brief interal rendering resolution
	*/
	const VkExtent2D renderResolution;

	/**
	* @brief display resolution
	*/
	const VkExtent2D presentResolution;

	/**
	* @brief name of window
	*/
	const std::string windowName;

	/**
	* @brief whether validations layers are enabled
	*/
	bool validationLayersEnabled;

	/**
	* @brief enabled validation layers
	*/
	const std::vector<const char*> validationLayers;

	/**
	* @brief enabled extensions
	*/
	const std::vector<const char*> deviceExtensions;

};