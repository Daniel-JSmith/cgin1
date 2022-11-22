#define VMA_IMPLEMENTATION
#include "VulkanCore.h"
#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>
#include "InputSupport.h"

std::unordered_map<AccessSpecifier::OPERATION, VkDescriptorType> VulkanCore::descriptorTypes
{
	{AccessSpecifier::OPERATION::COLOR_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
	{AccessSpecifier::OPERATION::DEPTH_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
	{AccessSpecifier::OPERATION::UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
};

VkDevice& VulkanCore::getDevice()
{
	return device;
}

VkPhysicalDevice VulkanCore::getPhysicalDevice()
{
	return physicalDevice;
}

VkSurfaceKHR VulkanCore::getSurface()
{
	return surface;
}

uint32_t VulkanCore::getGraphicsQueueFamilyIndex()
{
	return graphicsQueueFamilyIndex;
}
uint32_t VulkanCore::getPresentQueueFamilyIndex()
{
	return presentQueueFamilyIndex;
}

VkQueue VulkanCore::getGraphicsQueue()
{
	return graphicsQueue;
}

VkQueue VulkanCore::getPresentQueue()
{
	return presentQueue;
}

bool VulkanCore::engineRunning()
{
	return !glfwWindowShouldClose(VulkanCore::window);
}

VkExtent2D VulkanCore::getWindowResolution()
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	return VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

void VulkanCore::createCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = VulkanCore::getGraphicsQueueFamilyIndex();

	if (vkCreateCommandPool(VulkanCore::getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool");
	}
}

void VulkanCore::setupInstantCommands()
{
	// allocate command buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(VulkanCore::getDevice(), &allocInfo, &instantBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}

	// create fence
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	vkCreateFence(VulkanCore::getDevice(), &fenceCreateInfo, VK_NULL_HANDLE, &instantBufferReady);
}

VulkanCore::VulkanCore(const VkExtent2D & renderResolution, const VkExtent2D & presentResolution, const std::string & windowName, const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions) : renderResolution(renderResolution), presentResolution(presentResolution), windowName(windowName), validationLayersEnabled(validationLayers.size() > 0), validationLayers(validationLayers), deviceExtensions(deviceExtensions)
{
	initWindow();

	createInstance();
	setupDebugMessenger();

	createSurface();
	pickPhysicalDevice();

	createLogicalDevice();

	initVmaAllocator();

	createCommandPool();
	setupInstantCommands();
}

VulkanCore::~VulkanCore()
{
	vkFreeCommandBuffers(VulkanCore::getDevice(), commandPool, 1, &instantBuffer);
	vkDestroyCommandPool(VulkanCore::getDevice(), commandPool, nullptr);

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	vmaDestroyAllocator(vmaAllocator);

	glfwDestroyWindow(window);
	glfwTerminate();
}

uint32_t VulkanCore::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type");
}

bool foundQueueFamilies(std::vector<uint32_t> queueFamilyIndices)
{
	for (const auto& index : queueFamilyIndices)
	{
		if (index == -1)
		{
			return false;
		}
	}

	return true;
}

void findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t& graphicsQueueFamilyIndex, uint32_t& presentQueueFamilyIndex)
{
	graphicsQueueFamilyIndex = presentQueueFamilyIndex = -1;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilyCount && !foundQueueFamilies({ graphicsQueueFamilyIndex, presentQueueFamilyIndex }); i++)
	{
		// check graphics queue support
		if (queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueueFamilyIndex = i;
		}

		// check presentation queue support
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (presentSupport)
		{
			presentQueueFamilyIndex = i;
		}
	}
}

bool VulkanCore::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

VkResult VulkanCore::CreateDebugUtilsMessengerEXT(VkInstance instance
	, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
	, const VkAllocationCallbacks* pAllocator
	, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VulkanCore::DestroyDebugUtilsMessengerEXT(VkInstance instance
	, VkDebugUtilsMessengerEXT debugMessenger
	, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func)
	{
		func(instance, debugMessenger, pAllocator);
	}
}



bool VulkanCore::isDeviceSuitable(VkPhysicalDevice device)
{
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	findQueueFamilies(device, surface, graphicsQueueFamilyIndex, presentQueueFamilyIndex);
	return foundQueueFamilies({graphicsQueueFamilyIndex, presentQueueFamilyIndex}) && extensionsSupported;
}

void VulkanCore::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vInstance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vInstance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find suitable GPU");
	}
}

bool VulkanCore::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

std::vector<const char*> VulkanCore::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (validationLayersEnabled)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VulkanCore::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
}

void VulkanCore::createInstance()
{
	if (validationLayersEnabled && !checkValidationLayerSupport())
	{
		throw std::runtime_error("requested validation layers unavailable");
	}

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = VK_NULL_HANDLE;


	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensionsAvailable(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsAvailable.data());

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (validationLayersEnabled)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}



	if (vkCreateInstance(&createInfo, nullptr, &vInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create Vulkan instance");
	}
}

void VulkanCore::createSurface()
{
	if (glfwCreateWindowSurface(vInstance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface");
	}
}

void VulkanCore::createLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		graphicsQueueFamilyIndex,
		presentQueueFamilyIndex
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	VkPhysicalDeviceFeatures requestedFeatures{};

	VkPhysicalDeviceFeatures availableFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &availableFeatures);

	createInfo.pEnabledFeatures = &requestedFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (validationLayersEnabled)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device");
	}

	vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue);
}

void VulkanCore::initVmaAllocator()
{
	VmaAllocatorCreateInfo createInfo{};
	createInfo.physicalDevice = physicalDevice;
	createInfo.device = device;
	createInfo.instance = vInstance;

	vmaCreateAllocator(&createInfo, &vmaAllocator);
}

void VulkanCore::setupDebugMessenger()
{
	if (!validationLayersEnabled)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);


	if (CreateDebugUtilsMessengerEXT(vInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger");
	}
}







void VulkanCore::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(presentResolution.width, presentResolution.height, windowName.c_str(), nullptr, nullptr);

	InputSupport::initialize(window);
}

void VulkanCore::createDescriptorPool(std::vector<VkDescriptorType> descriptorsUsed, int numPasses)
{
	std::unordered_map<VkDescriptorType, int> numInstancesPerType;
	for (const auto& type : descriptorsUsed)
	{
		if (numInstancesPerType.count(type) > 0)
		{
			++numInstancesPerType[type];
		}
		else
		{
			numInstancesPerType[type] = 1;
		}
	}

	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const auto& pair : numInstancesPerType)
	{
		VkDescriptorPoolSize size{};
		size.type = pair.first;
		size.descriptorCount = pair.second;

		poolSizes.push_back(size);
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(numPasses);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool");
	}
}

VkDescriptorPool VulkanCore::getDescriptorPool()
{
	if (descriptorPool == VK_NULL_HANDLE)
	{
		throw std::runtime_error("descriptor pool not created");
	}

	return descriptorPool;
}

VmaAllocator VulkanCore::getVmaAllocator()
{
	return vmaAllocator;
}

void VulkanCore::executeInstantCommands(std::function<void(VkCommandBuffer)> commands)
{
	VkCommandPoolResetFlags flags{};
	vkResetCommandPool(VulkanCore::getDevice(), commandPool, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(instantBuffer, &beginInfo);

	commands(instantBuffer);

	vkEndCommandBuffer(instantBuffer);

	submitCommandBuffer(instantBuffer, instantBufferReady);

	// wait until commands are complete
	vkWaitForFences(VulkanCore::getDevice(), 1, &instantBufferReady, VK_TRUE, UINT64_MAX);
	vkResetFences(VulkanCore::getDevice(), 1, &instantBufferReady);
}

const VkExtent2D& VulkanCore::getRenderResolution() const
{
	return renderResolution;
}

void VulkanCore::submitCommandBuffer(VkCommandBuffer& commandBuffer, VkFence signalFence)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = VK_NULL_HANDLE;

	vkQueueSubmit(VulkanCore::getGraphicsQueue(), 1, &submitInfo, signalFence);
}

VkCommandPool VulkanCore::getCommandPool()
{
	return commandPool;
}