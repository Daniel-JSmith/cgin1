#include "Buffer.h"
#include <stdexcept>

const std::unordered_map<Resource::ACCESS_PROPERTY, void (Buffer::*)(VkDeviceSize, const void*)> Buffer::DATA_TRANSFER_FUNCTIONS =
{
	{Resource::ACCESS_PROPERTY::CPU_PREFERRED, &copyDataDirect},
	{Resource::ACCESS_PROPERTY::GPU_PREFERRED, &copyDataStaging}
};

const std::unordered_map<Resource::ACCESS_PROPERTY, VkBufferUsageFlags> Buffer::ACCESS_PROPERTY_USAGE_FLAGS =
{
	{Resource::ACCESS_PROPERTY::CPU_PREFERRED, 0},
	{Resource::ACCESS_PROPERTY::GPU_PREFERRED, VK_BUFFER_USAGE_TRANSFER_DST_BIT}
};

const std::unordered_map<AccessSpecifier::OPERATION, VkBufferUsageFlags> Buffer::USE_USAGE_FLAGS =
{
	{AccessSpecifier::OPERATION::VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
	{AccessSpecifier::OPERATION::INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
	{AccessSpecifier::OPERATION::UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_BUFFER, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT},
	{AccessSpecifier::OPERATION::TRANSFER_SOURCE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT},
	{AccessSpecifier::OPERATION::TRANSFER_DESTINATION, VK_BUFFER_USAGE_TRANSFER_DST_BIT},
};

VkBufferUsageFlags Buffer::getUsageFlags(Resource::ACCESS_PROPERTY accessProperty, AccessSpecifier::OPERATION use)
{
	return ACCESS_PROPERTY_USAGE_FLAGS.at(accessProperty) | USE_USAGE_FLAGS.at(use);
}

void Buffer::copyDataDirect(VkDeviceSize bufferSize, const void* data)
{
	void* destinationData;

	vmaMapMemory(getVulkanCoreSupport().getVmaAllocator(), bufferAllocation, &destinationData);
	memcpy(destinationData, data, (size_t)bufferSize);
	vmaUnmapMemory(getVulkanCoreSupport().getVmaAllocator(), bufferAllocation);
}

void Buffer::copyDataStaging(VkDeviceSize bufferSize, const void* data)
{
	// create staging buffer, copy data to staging buffer, copy staging buffer to this buffer

	// create temp staging buffer
	Buffer stagingBuffer(getVulkanCoreSupport(), byteSize, AccessSpecifier::OPERATION::TRANSFER_SOURCE, ACCESS_PROPERTY::CPU_PREFERRED, data);
	stagingBuffer.initialize();

	// copy data to GPU buffer
	copyBuffer(stagingBuffer);
}

void Buffer::setDataTransferFunction(Resource::ACCESS_PROPERTY accessProperty)
{
	dataTransferFunction = DATA_TRANSFER_FUNCTIONS.at(accessProperty);
}

void Buffer::copyData(VkDeviceSize size, const void* data)
{
	waitForReady();

	(*this.*dataTransferFunction)(size, data);
}

void Buffer::copyBuffer(const Buffer& otherBuffer)
{
	auto command = [&otherBuffer, this](VkCommandBuffer commandBuffer)
	{
		VkBufferCopy copyRegion{};
		copyRegion.size = otherBuffer.byteSize;
		vkCmdCopyBuffer(commandBuffer, otherBuffer.bufferObject, bufferObject, 1, &copyRegion);
	};

	getVulkanCoreSupport().executeInstantCommands(command);
}

Buffer::Buffer(VulkanCore& vulkanCoreSupport, VkDeviceSize size, AccessSpecifier::OPERATION use, ACCESS_PROPERTY accessProperty, const void* data) : byteSize(size), Resource(vulkanCoreSupport)
{
	setDataTransferFunction(accessProperty);

	initializeFunction = [this, accessProperty, use, size, data]()
	{
		createBuffer(accessProperty, use);
		copyData(size, data);
	};
}


Buffer::Buffer(VulkanCore& vulkanCoreSupport, VkDeviceSize size, AccessSpecifier::OPERATION use, ACCESS_PROPERTY accessProperty) : byteSize(size), Resource(vulkanCoreSupport)
{
	setDataTransferFunction(accessProperty);


	initializeFunction = [this, accessProperty, use]()
	{
		createBuffer(accessProperty, use);
	};
}

Buffer::~Buffer()
{
	vmaDestroyBuffer(getVulkanCoreSupport().getVmaAllocator(), bufferObject, bufferAllocation);
};


const VkBuffer& Buffer::getBufferObject() const
{
	return bufferObject;
}

// TODO behavior on non-descriptor resources
void Buffer::createLayoutBinding(int index, AccessSpecifier access, VkDescriptorSetLayoutBinding& out) const
{
	out.binding = index;
	out.descriptorType = DESCRIPTOR_TYPES.at(access.operation);
	out.descriptorCount = 1;
	out.stageFlags = STAGE_FLAGS.at(access.stage);
	out.pImmutableSamplers = nullptr;
}

void Buffer::createDescriptorWrite(int index, AccessSpecifier access, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const
{
	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = bufferObject;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = byteSize;

	out.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	out.dstSet = descriptorSet;
	out.dstBinding = index;
	out.dstArrayElement = 0;
	out.descriptorType = DESCRIPTOR_TYPES.at(access.operation);
	out.descriptorCount = 1;
	out.pBufferInfo = &descriptorBufferInfo;
	out.pImageInfo = nullptr;
	out.pTexelBufferView = nullptr;
	out.pNext = nullptr;

	vkUpdateDescriptorSets(getVulkanCoreSupport().getDevice(), 1, &out, 0, nullptr);
}

void Buffer::createBuffer(Resource::ACCESS_PROPERTY accessProperty, AccessSpecifier::OPERATION use)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = byteSize;
	bufferInfo.usage = getUsageFlags(accessProperty, use);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocationInfo{};
	allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;

	vmaCreateBuffer(getVulkanCoreSupport().getVmaAllocator(), &bufferInfo, &allocationInfo, &bufferObject, &bufferAllocation, nullptr);
}

void Buffer::prepareForInitialAccess(VkCommandBuffer& commandBuffer, AccessSpecifier currentAccess)
{

}

void Buffer::insertBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier previousAccess, AccessSpecifier currentAccess)
{
	auto previousAccessMask = getAccessFlag(previousAccess);
	auto currentAccessMask = getAccessFlag(currentAccess);

	auto previousStageMask = getPipelineStageFlag(previousAccess);
	auto currentStageMask = getPipelineStageFlag(currentAccess);

	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = previousAccessMask;
	barrier.dstAccessMask = currentAccessMask;

	vkCmdPipelineBarrier(
		commandBuffer,
		previousStageMask,
		currentStageMask,
		0,
		1, &barrier,
		0, VK_NULL_HANDLE,
		0, VK_NULL_HANDLE
	);

	auto x = 2;
}
