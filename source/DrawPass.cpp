#include "DrawPass.h"

#include <array>
#include <fstream>

DrawPass::DrawPass(VulkanCore& vulkanCoreSupport, std::vector<ResourceShaderInterface> resources, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const GeometryContainer& mesh) : PipelinePass(vulkanCoreSupport, resources), mesh(mesh), vertexShader(vulkanCoreSupport, vertexShaderPath), fragmentShader(vulkanCoreSupport, fragmentShaderPath)
{
	// separate output attachments now for easy access
	for (const ResourceShaderInterface& resource : resources)
	{
		if (resource.resource.accessSpecifier.operation == AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT
			|| resource.resource.accessSpecifier.operation == AccessSpecifier::OPERATION::DEPTH_BUFFER)
		{
			outputAttachments.push_back(resource);
		}
	}

}

DrawPass::~DrawPass()
{
	vkDestroyRenderPass(getVulkanCoreSupport().getDevice(), renderPass, nullptr);
	vkDestroyFramebuffer(getVulkanCoreSupport().getDevice(), frameBuffer, nullptr);
}

void DrawPass::prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	PipelinePass::prepareExecution(insertBarriers);
	createRenderPass();
	createFrameBuffer();
	createPipeline();
	recordCommandBuffer(insertBarriers);
}

inline bool isColorAttachment(ResourceAccessSpecifier access)
{
	return access.accessSpecifier.operation == AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT;
}
inline bool isColorAttachment(ResourceShaderInterface access)
{
	return isColorAttachment(access.resource);
}

void DrawPass::createRenderPass()
{
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	std::vector<VkAttachmentReference> colorAttachmentReferences;
	VkAttachmentReference depthAttachmentReference;

	for (uint32_t i = 0; i < outputAttachments.size(); i++)
	{
		VkAttachmentDescription description;
		VkAttachmentReference reference;

		static_cast<Image*>(outputAttachments.at(i).resource.resource)->getAttachmentDescription(outputAttachments.at(i).resource.accessSpecifier, i, description, reference, outputAttachments.at(i).blendEnabled);

		attachmentDescriptions.push_back(description);
		if (isColorAttachment(outputAttachments.at(i)))
		{
			colorAttachmentReferences.push_back(reference);
		}
		else
		{
			depthAttachmentReference = reference;
		}
	}
	
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
	subpass.pColorAttachments = colorAttachmentReferences.data();
	// TODO handle the case of a pass without a depth attachment
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = VK_NULL_HANDLE;

	if (vkCreateRenderPass(getVulkanCoreSupport().getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass");
	}


}

void DrawPass::createFrameBuffer()
{
	std::vector<VkImageView> attachmentViews;
	for (const auto& outputAttachment : outputAttachments)
	{
		attachmentViews.push_back(static_cast<Image*>(outputAttachment.resource.resource)->getImageView());
	}

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(outputAttachments.size());
	framebufferInfo.pAttachments = attachmentViews.data();
	framebufferInfo.width = static_cast<Image*>(outputAttachments.at(0).resource.resource)->getExtent().width;
	framebufferInfo.height = static_cast<Image*>(outputAttachments.at(0).resource.resource)->getExtent().height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(getVulkanCoreSupport().getDevice(), &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create frambuffer");
	}
}

void DrawPass::createPipeline()
{
	VkExtent2D extent = static_cast<Image*>(outputAttachments.at(0).resource.resource)->getExtent();

	// vertex shader
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexShader.getModule();
	vertShaderStageInfo.pName = "main";

	// fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragmentShader.getModule();
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// vertex shader input
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions = mesh.getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	auto bindingDescription = mesh.getBindingDescription();
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// scissor
	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = extent;

	// viewport state
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// rasterzation state
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = mesh.getWindingOrder();
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	// multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// color blending
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	for (const ResourceShaderInterface& outputAttachment : outputAttachments)
	{
		if (isColorAttachment(outputAttachment))
		{
			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask =
				VK_COLOR_COMPONENT_R_BIT
				| VK_COLOR_COMPONENT_G_BIT
				| VK_COLOR_COMPONENT_B_BIT
				| VK_COLOR_COMPONENT_A_BIT;
			
			
			
			
			
			
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.blendEnable = outputAttachment.blendEnabled;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			colorBlendAttachments.push_back(colorBlendAttachment);
		}
	}

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
	colorBlending.pAttachments = colorBlendAttachments.data();

	// depth buffering
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;

	pipelineInfo.layout = pipelineLayout;

	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(getVulkanCoreSupport().getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline");
	}
}

void DrawPass::recordCommandBuffer(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	startCommandBufferRecording(insertBarriers);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = frameBuffer;
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = getVulkanCoreSupport().getRenderResolution();

	std::vector<VkClearValue> clearValues;
	for (const ResourceShaderInterface& outputAttachment : outputAttachments)
	{
		VkClearValue clearValue{};

		if (isColorAttachment(outputAttachment))
		{
			clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		}
		else // is depth attachment
		{
			clearValue.depthStencil = { 1.0f, 0 };
		}

		clearValues.push_back(clearValue);
	}

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkBuffer vertexBuffers[] = { mesh.getVertexBuffer().getBufferObject() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);



	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	vkCmdBindIndexBuffer(commandBuffer, mesh.getIndexBuffer().getBufferObject(), 0, mesh.getIndexType());

	vkCmdDrawIndexed(commandBuffer, mesh.getNumIndices(), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	endCommandBufferRecording();
}
