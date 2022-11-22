#include "ClearPass.h"

ClearPass::ClearPass(VulkanCore& vulkanCoreSupport, Image& image) : Pass(vulkanCoreSupport, { ResourceShaderInterface{ResourceAccessSpecifier{ &image, AccessSpecifier{AccessSpecifier::OPERATION::CLEAR_OUTSIDE_RENDER_PASS, AccessSpecifier::STAGE::TRANSFER} }} }), image(image)
{

}

void ClearPass::prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	Pass::prepareExecution(insertBarriers);
	recordCommandBuffer(insertBarriers);
}

void ClearPass::recordCommandBuffer(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	startCommandBufferRecording(insertBarriers);

	VkClearColorValue clearColorValue{ 0, 0, 0, 0};

	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;

	image.prepareForInitialAccess(commandBuffer, AccessSpecifier{ AccessSpecifier::OPERATION::CLEAR_OUTSIDE_RENDER_PASS, AccessSpecifier::STAGE::TRANSFER });

	vkCmdClearColorImage(commandBuffer, image.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &range);

	endCommandBufferRecording();
}