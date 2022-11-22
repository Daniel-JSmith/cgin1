#pragma once
#include <vector>
#include <functional>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Image.h"
#include "VulkanCore.h"
#include "ResourceAccessSpecifier.h"

/**
* @brief Representation of an execution of a GPU operation: some number of graphics API commands, often including execution of a shader.
* 
* A Pass object is a record of GPU operation. It consists of an operation and one or more affected Resource objects. Passes are usually declared beforehand and executed many times, in the manner of Vulkan command buffers.
*/
class Pass
{
public:

	/**
	* @brief Creates a Pass using the specified resources.
	* 
	* @param resources what resources the pass will use and how
	*/
	Pass(VulkanCore& vulkanCoreSupport, std::vector<ResourceShaderInterface> resources);

	/**
	* @brief Destroys this Pass.
	*/
	virtual ~Pass();

	/**
	* @brief Completes initialization required for executing this Pass.
	* 
	* @param insertBarriers function recording GPU API synchronization commands
	*/
	virtual void prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers);

	/**
	* @brief Submits pass' commands to GPU.
	* 
	*/
	void execute();

	/**
	* @brief Returns underlying Vulkan command buffer object.
	* 
	* @return handle to Vulkan command buffer object
	*/
	VkCommandBuffer& getCommandBuffer();

	/**
	* @brief Returns resources accessed in this Pass.
	* 
	* @param output vector to fill with pass resources
	*/
	void getResources(std::vector<ResourceAccessSpecifier>& output) const;

protected:

	/**
	* @brief handle to Vulkan descriptor set layout
	*/
	VkDescriptorSetLayout descriptorSetLayout;

	/**
	* @brief handle to Vulkan descriptor set
	*/
	VkDescriptorSet descriptorSet;

	/**
	* @brief handle to Vulkan command buffer
	*/
	VkCommandBuffer commandBuffer;

	/**
	* @brief Prepares GPU command buffer for recording. Assumes called before endCommandBufferRecording.
	* 
	* @param insertBarriers function recording GPU API synchronization commands
	*/
	void startCommandBufferRecording(std::function<void(VkCommandBuffer, Pass*)> insertBarriers);

	/**
	* @brief Ends GPU command buffer recording. Assumes called after startCommandBufferRecording.
	* 
	*/
	void endCommandBufferRecording();

	/**
	* @brief Records GPU commands.
	* 
	* @param insertBarriers function recording GPU API synchronization commands
	*/
	virtual void recordCommandBuffer(std::function<void(VkCommandBuffer, Pass*)> insertBarriers) = 0;

	VulkanCore& getVulkanCoreSupport();

private:

	VulkanCore& vulkanCoreSupport;

	std::vector<ResourceShaderInterface> resources;

	VkFence notExecuting;

	void createDescriptorSetLayout();
	void createDescriptorSet();

	void allocateCommandBuffer();

};