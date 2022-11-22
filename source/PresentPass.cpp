#include "PresentPass.h"

PresentPass::PresentPass(VulkanCore& vulkanCoreSupport, Image& sourceImage, Image& swapChainImage) : Pass(vulkanCoreSupport, { ResourceShaderInterface{ ResourceAccessSpecifier{&sourceImage, AccessSpecifier{AccessSpecifier::OPERATION::PREPARE_FOR_PRESENTATION, AccessSpecifier::STAGE::TRANSFER}}}, ResourceShaderInterface{ResourceAccessSpecifier{&swapChainImage, AccessSpecifier{AccessSpecifier::OPERATION::PRESENT, AccessSpecifier::STAGE::TRANSFER}}} }), sourceImage(sourceImage), swapChainImage(swapChainImage)
{

}

PresentPass::~PresentPass()
{

}

void PresentPass::prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	Pass::prepareExecution(insertBarriers);
	recordCommandBuffer(insertBarriers);
}

void PresentPass::recordCommandBuffer(std::function<void(VkCommandBuffer, Pass*)> insertBarriers)
{
	startCommandBufferRecording(insertBarriers);

	swapChainImage.prepareForInitialAccess(commandBuffer, AccessSpecifier{ AccessSpecifier::OPERATION::TRANSFER_DESTINATION, AccessSpecifier::STAGE::TRANSFER });

	VkImageSubresourceLayers subresourceLayers{};
	subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceLayers.mipLevel = 0;
	subresourceLayers.baseArrayLayer = 0;
	subresourceLayers.layerCount = 1;

	VkImageBlit blitRegion{};
	blitRegion.srcSubresource = subresourceLayers;
	blitRegion.srcOffsets[0] = { 0, 0, 0 }; blitRegion.srcOffsets[1] = { static_cast<int32_t>(sourceImage.getExtent().width), static_cast<int32_t>(sourceImage.getExtent().height), 1 };
	blitRegion.dstSubresource = subresourceLayers;
	blitRegion.dstOffsets[0] = { 0, 0, 0 }; blitRegion.dstOffsets[1] = { static_cast<int32_t>(swapChainImage.getExtent().width), static_cast<int32_t>(swapChainImage.getExtent().height), 1 };

	vkCmdBlitImage(commandBuffer, sourceImage.getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapChainImage.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_NEAREST);

	swapChainImage.insertBarrier(commandBuffer, AccessSpecifier{ AccessSpecifier::OPERATION::TRANSFER_DESTINATION, AccessSpecifier::STAGE::TRANSFER }, AccessSpecifier{ AccessSpecifier::OPERATION::PRESENT, AccessSpecifier::STAGE::TRANSFER });

	endCommandBufferRecording();
}