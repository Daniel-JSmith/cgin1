#include "ComputePass.h"

ComputePass::ComputePass(VulkanCore& vulkanCoreSupport, std::vector<ResourceShaderInterface> resources, const std::string& computeShaderPath) : PipelinePass(vulkanCoreSupport, resources), computeShader(vulkanCoreSupport, computeShaderPath)
{
}

ComputePass::~ComputePass()
{
}

void ComputePass::prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	PipelinePass::prepareExecution(insertBarriers);
	createPipeline();
	recordCommandBuffer(insertBarriers);
}

void ComputePass::createPipeline()
{
	VkPipelineShaderStageCreateInfo stageCreateInfo;
	stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageCreateInfo.pNext = VK_NULL_HANDLE;
	stageCreateInfo.flags = 0;
	stageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageCreateInfo.module = computeShader.getModule();
	stageCreateInfo.pName = "main";
	stageCreateInfo.pSpecializationInfo = VK_NULL_HANDLE;
	
	VkComputePipelineCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	createInfo.pNext = VK_NULL_HANDLE;
	createInfo.flags = 0;
	createInfo.stage = stageCreateInfo;
	createInfo.layout = pipelineLayout;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = 0;

	if (vkCreateComputePipelines(getVulkanCoreSupport().getDevice(), VK_NULL_HANDLE, 1, &createInfo, VK_NULL_HANDLE, &pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create compute pass");
	}
}

void ComputePass::recordCommandBuffer(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	startCommandBufferRecording(insertBarriers);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	vkCmdDispatch(commandBuffer, getVulkanCoreSupport().getRenderResolution().width / 16 + 1, getVulkanCoreSupport().getRenderResolution().height / 16 + 1, 1);

	endCommandBufferRecording();
}