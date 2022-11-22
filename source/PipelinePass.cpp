#include "PipelinePass.h"

PipelinePass::PipelinePass(VulkanCore& vulkanCoreSupport, std::vector<ResourceShaderInterface> resources) : Pass(vulkanCoreSupport, resources)
{
	createPipelineLayout();
}

PipelinePass::~PipelinePass()
{
	vkDeviceWaitIdle(getVulkanCoreSupport().getDevice());
	vkDestroyPipelineLayout(getVulkanCoreSupport().getDevice(), pipelineLayout, nullptr);
	vkDestroyPipeline(getVulkanCoreSupport().getDevice(), pipeline, nullptr);
}

void PipelinePass::prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	Pass::prepareExecution(insertBarriers);
}

void PipelinePass::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = VK_NULL_HANDLE;

	if (vkCreatePipelineLayout(getVulkanCoreSupport().getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout");
	}
}