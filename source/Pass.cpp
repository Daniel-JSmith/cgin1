#include "Pass.h"

#include <array>
#include <fstream>

Pass::Pass(VulkanCore& vulkanCoreSupport, std::vector<ResourceShaderInterface> resources) : vulkanCoreSupport(vulkanCoreSupport), resources(resources)
{
	// create fence
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (vkCreateFence(vulkanCoreSupport.getDevice(), &fenceInfo, nullptr, &notExecuting) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor fence");
	}

	// register fence with descriptors
	for (ResourceShaderInterface& resourceAccess : resources)
	{
		resourceAccess.resource.resource->registerResourceUse(notExecuting, resourceAccess.resource.accessSpecifier);
	}

	createDescriptorSetLayout();
	allocateCommandBuffer();
}

Pass::~Pass()
{
	VkDevice& device = vulkanCoreSupport.getDevice();

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkFreeCommandBuffers(device, vulkanCoreSupport.getCommandPool(), 1, &commandBuffer);
}

void Pass::prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	createDescriptorSet();
}

void Pass::execute()
{
	// set this pass to executing
	vkWaitForFences(vulkanCoreSupport.getDevice(), 1, &notExecuting, VK_TRUE, UINT64_MAX);
	vkResetFences(vulkanCoreSupport.getDevice(), 1, &notExecuting);

	vulkanCoreSupport.submitCommandBuffer(commandBuffer, notExecuting);
}

void Pass::createDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (const ResourceShaderInterface& resource : resources)
	{
		if (resource.isDescriptor())
		{
			VkDescriptorSetLayoutBinding layoutBinding;
			resource.resource.resource->createLayoutBinding(resource.descriptorBinding, resource.resource.accessSpecifier, layoutBinding);
			bindings.push_back(layoutBinding);
		}
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(vulkanCoreSupport.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout");
	}
}

void Pass::createDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = vulkanCoreSupport.getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;


	VkResult result = vkAllocateDescriptorSets(vulkanCoreSupport.getDevice(), &allocInfo, &descriptorSet);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	for (const ResourceShaderInterface& resource : resources)
	{
		if (resource.isDescriptor())
		{
			VkWriteDescriptorSet descriptorWrite;
			resource.resource.resource->createDescriptorWrite(resource.descriptorBinding, resource.resource.accessSpecifier, descriptorSet, descriptorWrite);
		}
	}
}

void Pass::allocateCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vulkanCoreSupport.getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(vulkanCoreSupport.getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}
}

void Pass::startCommandBufferRecording(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	insertBarriers(commandBuffer, this);
}

VulkanCore& Pass::getVulkanCoreSupport()
{
	return vulkanCoreSupport;
}

void Pass::endCommandBufferRecording()
{
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer");
	}
}

VkCommandBuffer& Pass::getCommandBuffer()
{
	return commandBuffer;
}

void Pass::getResources(std::vector<ResourceAccessSpecifier>& output) const
{
	output.clear();

	for (ResourceShaderInterface resource : resources)
	{
		output.push_back(resource.resource);
	}
}