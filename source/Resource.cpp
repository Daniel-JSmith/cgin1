#include "Resource.h"

const std::unordered_map<Resource::ACCESS_PROPERTY, VkMemoryPropertyFlags> Resource::MEMORY_PROPERTY_FLAGS =
{
	{Resource::ACCESS_PROPERTY::CPU_PREFERRED, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
	{Resource::ACCESS_PROPERTY::GPU_PREFERRED, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
};

// TODO duplicated in VulkanCoreSupport
const std::unordered_map<AccessSpecifier::OPERATION, VkDescriptorType> Resource::DESCRIPTOR_TYPES =
{
	{AccessSpecifier::OPERATION::DEPTH_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
	{AccessSpecifier::OPERATION::COLOR_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},

	{AccessSpecifier::OPERATION::UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}
};

const std::unordered_map<AccessSpecifier::STAGE, VkShaderStageFlagBits> Resource::STAGE_FLAGS
{
	{AccessSpecifier::STAGE::VERTEX_SHADER, VK_SHADER_STAGE_VERTEX_BIT},
	{AccessSpecifier::STAGE::FRAGMENT_SHADER, VK_SHADER_STAGE_FRAGMENT_BIT},
	{AccessSpecifier::STAGE::COMPUTE_SHADER, VK_SHADER_STAGE_COMPUTE_BIT}
};

const std::unordered_map<AccessSpecifier::OPERATION, VkAccessFlags> Resource::ACCESS_FLAGS
{
	{AccessSpecifier::OPERATION::NO_OPERATION, VK_ACCESS_NONE},
	{AccessSpecifier::OPERATION::COLOR_SAMPLER, VK_ACCESS_SHADER_READ_BIT},
	{AccessSpecifier::OPERATION::DEPTH_SAMPLER, VK_ACCESS_SHADER_READ_BIT},
	{AccessSpecifier::OPERATION::PREPARE_FOR_PRESENTATION, VK_ACCESS_TRANSFER_READ_BIT},
	{AccessSpecifier::OPERATION::PRESENT, VK_ACCESS_TRANSFER_WRITE_BIT},
	{AccessSpecifier::OPERATION::TRANSFER_SOURCE, VK_ACCESS_TRANSFER_READ_BIT},
	{AccessSpecifier::OPERATION::TRANSFER_DESTINATION, VK_ACCESS_TRANSFER_WRITE_BIT},
	{AccessSpecifier::OPERATION::VERTEX_BUFFER, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT},
	{AccessSpecifier::OPERATION::INDEX_BUFFER, VK_ACCESS_INDEX_READ_BIT},
	{AccessSpecifier::OPERATION::UNIFORM_BUFFER, VK_ACCESS_UNIFORM_READ_BIT},
	{AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_BUFFER, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT},
	{AccessSpecifier::OPERATION::DEPTH_BUFFER, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_IMAGE, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT},
	{AccessSpecifier::OPERATION::CLEAR_OUTSIDE_RENDER_PASS, VK_ACCESS_TRANSFER_WRITE_BIT}
	
};

const std::unordered_map<AccessSpecifier::STAGE, VkPipelineStageFlagBits> Resource::PIPELINE_STAGE_FLAGS
{
	{AccessSpecifier::STAGE::INITIAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
	{AccessSpecifier::STAGE::VERTEX_SHADER, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT},
	{AccessSpecifier::STAGE::FRAGMENT_SHADER, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
	{AccessSpecifier::STAGE::COMPUTE_SHADER, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT},
	{AccessSpecifier::STAGE::TRANSFER, VK_PIPELINE_STAGE_TRANSFER_BIT},
	{AccessSpecifier::STAGE::FINAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}
};

Resource::Resource(VulkanCore& vulkanCoreSupport) : vulkanCoreSupport(vulkanCoreSupport)
{

}

void Resource::registerResourceUse(const VkFence& passFence, AccessSpecifier accessSpecifier)
{
	notInUseFences.push_back(passFence);
	accessTypes.insert(accessSpecifier.operation);
}

void Resource::waitForReady() const
{
	if (notInUseFences.size() > 0)
	{
		if (vkWaitForFences(vulkanCoreSupport.getDevice(), static_cast<uint32_t>(notInUseFences.size()), notInUseFences.data(), VK_TRUE, UINT64_MAX) != VK_SUCCESS)
		{
			throw std::runtime_error("wait for fence timeout");
		}
	}
}

void Resource::initialize()
{
	initializeFunction();
}

/**
* @return Vulkan access flag
*/
VkAccessFlags Resource::getAccessFlag(const AccessSpecifier& accessSpecifier)
{
	return ACCESS_FLAGS.at(accessSpecifier.operation);
}

/**
* @return Vulkan pipeline stage flag
*/
VkPipelineStageFlagBits Resource::getPipelineStageFlag(const AccessSpecifier& accessSpecifier)
{
	if (accessSpecifier.operation == AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT)
	{
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	if (accessSpecifier.operation == AccessSpecifier::OPERATION::DEPTH_BUFFER)
	{
		return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}

	return Resource::PIPELINE_STAGE_FLAGS.at(accessSpecifier.stage);
}

VulkanCore& Resource::getVulkanCoreSupport() const
{
	return vulkanCoreSupport;
}
