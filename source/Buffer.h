#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <unordered_map>
#include "VulkanCore.h"
#include "Resource.h"

/**
* @brief Representation of a GPU-accessible buffer
*/
class Buffer : public Resource
{
public:

	/**
	* @brief Creates a Buffer filled with data.
	* 
	* @param size size of buffer in bytes
	* @param use how buffer will be used
	* @param accessProperty memory location preference
	* @param data pointer to data to fill buffer with
	*/
	Buffer(VulkanCore& vulkanCoreSupport, VkDeviceSize size, AccessSpecifier::OPERATION use, ACCESS_PROPERTY accessProperty, const void* data);

	/**
	* @brief Creates an empty Buffer filled.
	*
	* @param size size of buffer in bytes
	* @param use how buffer will be used
	* @param accessProperty memory location preference
	*/
	Buffer(VulkanCore& vulkanCoreSupport, VkDeviceSize size, AccessSpecifier::OPERATION use, ACCESS_PROPERTY accessProperty);

	Buffer(const Buffer&) = delete;
	Buffer(Buffer&&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&&) = delete;
	
	/**
	* @brief Destroys this Buffer, freeing allocated memory.
	*/
	~Buffer();

	/**
	* @brief Returns underlying vulkan buffer object.
	* 
	* @return vulkan buffer object
	*/
	const VkBuffer& getBufferObject() const;
	
	/**
	* @brief Copies data to this Buffer.
	* 
	* @param size size of data to copy in bytes
	* @param data data to copy to buffer
	*/
	void copyData(VkDeviceSize size, const void* data);

	void createLayoutBinding(int index, AccessSpecifier access, VkDescriptorSetLayoutBinding& out) const override;
	void createDescriptorWrite(int index, AccessSpecifier access, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const override;

	void prepareForInitialAccess(VkCommandBuffer& commandBuffer, AccessSpecifier currentAccess) override;

	void insertBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier previousAccess, AccessSpecifier currentAccess) override;

private:
	static const std::unordered_map<ACCESS_PROPERTY, void (Buffer::*)(VkDeviceSize, const void*)> DATA_TRANSFER_FUNCTIONS;
	static const std::unordered_map<ACCESS_PROPERTY, VkBufferUsageFlags> ACCESS_PROPERTY_USAGE_FLAGS;
	static const std::unordered_map<AccessSpecifier::OPERATION, VkBufferUsageFlags> USE_USAGE_FLAGS;

	// size in bytes
	const VkDeviceSize byteSize;

	VkBuffer bufferObject;

	VmaAllocation bufferAllocation;

	void (Buffer::*dataTransferFunction)(VkDeviceSize, const void*);

	void setDataTransferFunction(Resource::ACCESS_PROPERTY accessProperty);

	VkBufferUsageFlags getUsageFlags(Resource::ACCESS_PROPERTY accessProperty, AccessSpecifier::OPERATION use);

	void createBuffer(Resource::ACCESS_PROPERTY accessProperty, AccessSpecifier::OPERATION use);
	
	void copyBuffer(const Buffer& otherBuffer);

	void copyDataDirect(VkDeviceSize bufferSize, const void* data);
	void copyDataStaging(VkDeviceSize bufferSize, const void* data);
};