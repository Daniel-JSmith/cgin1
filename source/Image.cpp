#include "Image.h"

#include <iostream>

#include "PixelDataContainer.h"

const std::unordered_map<AccessSpecifier::OPERATION, VkImageLayout> Image::REQUIRED_LAYOUTS = 
{
	{AccessSpecifier::OPERATION::NO_OPERATION, VK_IMAGE_LAYOUT_UNDEFINED},
	{AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
	{AccessSpecifier::OPERATION::COLOR_SAMPLER, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
	{AccessSpecifier::OPERATION::DEPTH_SAMPLER, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
	{AccessSpecifier::OPERATION::TRANSFER_DESTINATION, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
	{AccessSpecifier::OPERATION::TRANSFER_SOURCE, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
	{AccessSpecifier::OPERATION::PREPARE_FOR_PRESENTATION, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
	{AccessSpecifier::OPERATION::PRESENT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL},
	{AccessSpecifier::OPERATION::CLEAR_OUTSIDE_RENDER_PASS, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
	{AccessSpecifier::OPERATION::DEPTH_BUFFER, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
};

const std::unordered_map<AccessSpecifier::OPERATION, VkBufferUsageFlags> Image::USE_USAGE_FLAGS =
{
	{AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
	{AccessSpecifier::OPERATION::DEPTH_SAMPLER, VK_IMAGE_USAGE_SAMPLED_BIT},
	{AccessSpecifier::OPERATION::COLOR_SAMPLER, VK_IMAGE_USAGE_SAMPLED_BIT},
	{AccessSpecifier::OPERATION::TRANSFER_SOURCE, VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
	{AccessSpecifier::OPERATION::TRANSFER_DESTINATION, VK_IMAGE_USAGE_TRANSFER_DST_BIT},
	{AccessSpecifier::OPERATION::PREPARE_FOR_PRESENTATION, VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
	{AccessSpecifier::OPERATION::PRESENT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_IMAGE, VK_IMAGE_USAGE_STORAGE_BIT},
	{AccessSpecifier::OPERATION::DEPTH_BUFFER, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT},
	{AccessSpecifier::OPERATION::CLEAR_OUTSIDE_RENDER_PASS, VK_IMAGE_USAGE_TRANSFER_DST_BIT}
};

const std::unordered_map<AccessSpecifier::OPERATION, VkImageAspectFlags> Image::USE_ASPECT_FLAGS
{
	{AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT, VK_IMAGE_ASPECT_COLOR_BIT},
	{AccessSpecifier::OPERATION::DEPTH_SAMPLER, VK_IMAGE_ASPECT_DEPTH_BIT},
	{AccessSpecifier::OPERATION::COLOR_SAMPLER, VK_IMAGE_ASPECT_COLOR_BIT},
	{AccessSpecifier::OPERATION::TRANSFER_SOURCE, VK_IMAGE_ASPECT_COLOR_BIT},
	{AccessSpecifier::OPERATION::TRANSFER_DESTINATION, VK_IMAGE_ASPECT_COLOR_BIT},
	{AccessSpecifier::OPERATION::PREPARE_FOR_PRESENTATION, VK_IMAGE_ASPECT_COLOR_BIT},
	{AccessSpecifier::OPERATION::PRESENT, VK_IMAGE_ASPECT_COLOR_BIT},
	{AccessSpecifier::OPERATION::SHADER_STORAGE_IMAGE, VK_IMAGE_ASPECT_COLOR_BIT},
	{AccessSpecifier::OPERATION::CLEAR_OUTSIDE_RENDER_PASS, VK_IMAGE_ASPECT_COLOR_BIT},
	{AccessSpecifier::OPERATION::DEPTH_BUFFER, VK_IMAGE_ASPECT_DEPTH_BIT}
};

void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageView& imageView, VkDevice& device)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image view");
	}
}

void createSampler(VkSampler& sampler, VkDevice& device)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create sampler");
	}
}

// support for constructors
void Image::init(VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkExtent2D extent, VkImageAspectFlags aspect)
{
	this->format = format;
	this->extent = extent;
	this->memoryProperties = memoryProperties;

	// create image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = extent.width;
	imageInfo.extent.height = extent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

	VmaAllocationCreateInfo imageCreateInfo{};
	imageCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

	vmaCreateImage(getVulkanCoreSupport().getVmaAllocator(), &imageInfo, &imageCreateInfo, &image, &imageAllocation, nullptr);

	responsibleForImageDestruction = true;

	// create view
	createImageView(image, format, aspect, imageView, getVulkanCoreSupport().getDevice());

	// create sampler
	createSampler(sampler, getVulkanCoreSupport().getDevice());
}

void Image::initializeEmptyImage(VkExtent2D extent, VkFormat format, ACCESS_PROPERTY accessProperty)
{
	this->extent = extent;
	this->format = format;

	VkImageUsageFlags useFlags = 0;
	VkImageAspectFlags aspectFlags = 0;
	for (auto use : accessTypes)
	{
		useFlags |= USE_USAGE_FLAGS.at(use);
		aspectFlags |= USE_ASPECT_FLAGS.at(use);
	}


	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = extent.width;
	imageInfo.extent.height = extent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = useFlags;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

	VmaAllocationCreateInfo imageCreateInfo{};
	imageCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

	auto result = vmaCreateImage(getVulkanCoreSupport().getVmaAllocator(), &imageInfo, &imageCreateInfo, &image, &imageAllocation, nullptr);

	// create image
	responsibleForImageDestruction = true;

	// create view
	createImageView(image, format, aspectFlags, imageView, getVulkanCoreSupport().getDevice());

	// create sampler
	createSampler(sampler, getVulkanCoreSupport().getDevice());
}

Image::Image(VulkanCore& vulkanCoreSupport, VkFormat format, VkExtent2D extent, ACCESS_PROPERTY accessProperty) : Resource(vulkanCoreSupport)
{
	initializeFunction = [this, extent, format, accessProperty]()
	{
		initializeEmptyImage(extent, format, accessProperty);
	};
}

Image::Image(VulkanCore& vulkanCoreSupport, VkFormat format, const std::string& path, ACCESS_PROPERTY accessProperty) : Resource(vulkanCoreSupport)
{

	initializeFunction = [this, path, format, accessProperty]()
	{
		// read from texture file
		PixelDataContainer pixels(path);

		// Create temp stagingBuffer
		Buffer stagingBuffer(getVulkanCoreSupport(), pixels.getSizeInBytes(), AccessSpecifier::OPERATION::TRANSFER_SOURCE, ACCESS_PROPERTY::CPU_PREFERRED, pixels.getData());

		initializeEmptyImage(VkExtent2D{ static_cast<uint32_t>(pixels.getWidth()), static_cast<uint32_t>(pixels.getHeight()) }, format, accessProperty);

		copyBufferToImage(stagingBuffer);
	};
}

Image::Image(VulkanCore& vulkanCoreSupport, VkImage& image, VkFormat format, VkExtent2D extent) : Resource(vulkanCoreSupport)
{
	this->image = image;
	this->format = format;
	this->extent = extent;
	createImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, imageView, getVulkanCoreSupport().getDevice());
	createSampler(sampler, getVulkanCoreSupport().getDevice());

	initializeFunction = []()
	{
		
	};
}


Image::~Image()
{
	vkDestroySampler(getVulkanCoreSupport().getDevice(), sampler, nullptr);
	vkDestroyImageView(getVulkanCoreSupport().getDevice(), imageView, nullptr);

	if (responsibleForImageDestruction)
	{
		vmaDestroyImage(getVulkanCoreSupport().getVmaAllocator(), image, imageAllocation);
	}
}

void Image::copyBufferToImage(const Buffer& buffer)
{
	auto copyCommand = [&buffer, this](VkCommandBuffer commandBuffer)
	{
		// We assume this is only called in constructors TODO
		prepareForInitialAccess(commandBuffer, { AccessSpecifier::OPERATION::TRANSFER_DESTINATION, AccessSpecifier::STAGE::TRANSFER });

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			extent.width,
			extent.height,
			1
		};

		VkBuffer b = buffer.getBufferObject();

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer.getBufferObject(),
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);
	};

	getVulkanCoreSupport().executeInstantCommands(copyCommand);
}

void Image::createLayoutBinding(int index, AccessSpecifier access, VkDescriptorSetLayoutBinding& out) const
{
	out.binding = index;
	out.descriptorType = Resource::DESCRIPTOR_TYPES.at(access.operation);
	out.descriptorCount = 1;
	out.stageFlags = STAGE_FLAGS.at(access.stage);
	out.pImmutableSamplers = nullptr;
}

void Image::createDescriptorWrite(int index, AccessSpecifier access, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const
{
	VkDescriptorImageInfo samplerInfo{};
	samplerInfo.imageLayout = REQUIRED_LAYOUTS.at(access.operation);
	samplerInfo.imageView = imageView;
	samplerInfo.sampler = sampler;

	out.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	out.dstSet = descriptorSet;
	out.dstBinding = index;
	out.dstArrayElement = 0;
	out.descriptorType = Resource::DESCRIPTOR_TYPES.at(access.operation);
	out.descriptorCount = 1;
	out.pBufferInfo = nullptr;
	out.pImageInfo = &samplerInfo;
	out.pTexelBufferView = nullptr;
	out.pNext = nullptr;

	vkUpdateDescriptorSets(getVulkanCoreSupport().getDevice(), 1, &out, 0, nullptr);
}

void Image::prepareForInitialAccess(VkCommandBuffer& commandBuffer, AccessSpecifier currentAccess)
{
	insertBarrier(commandBuffer, AccessSpecifier{ AccessSpecifier::OPERATION::NO_OPERATION, AccessSpecifier::STAGE::INITIAL }, currentAccess);
}

void Image::insertBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier previousAccess, AccessSpecifier currentAccess)
{
	VkImageLayout currentLayout = REQUIRED_LAYOUTS.at(previousAccess.operation);
	VkImageLayout requiredLayout = REQUIRED_LAYOUTS.at(currentAccess.operation);

	auto previousAccessMask = getAccessFlag(previousAccess);
	auto currentAccessMask = getAccessFlag(currentAccess);

	auto previousStageMask = getPipelineStageFlag(previousAccess);
	auto currentStageMask = getPipelineStageFlag(currentAccess);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = previousAccessMask; //VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = currentAccessMask;  //VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.oldLayout = currentLayout;
	barrier.newLayout = requiredLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = USE_ASPECT_FLAGS.at(currentAccess.operation);;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		commandBuffer,
		previousStageMask, //VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		currentStageMask, //VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		0,
		0, VK_NULL_HANDLE,
		0, VK_NULL_HANDLE,
		1, &barrier
	);

	currentLayout = requiredLayout;
}

void Image::getAttachmentDescription(AccessSpecifier currentAccess, uint32_t attachmentNumber, VkAttachmentDescription& attachmentDescription, VkAttachmentReference& attachmentReference, bool blend)
{
	VkImageLayout requiredLayout = REQUIRED_LAYOUTS.at(currentAccess.operation);

	attachmentDescription = VkAttachmentDescription{};
	attachmentDescription.format = format;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

	if (!blend)
	{
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	else
	{
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescription.initialLayout = requiredLayout;
	}
	
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	
	// currently, we don't use the automatic layout transition from a renderpass ending
	attachmentDescription.finalLayout = requiredLayout;


	attachmentReference = VkAttachmentReference{};
	attachmentReference.attachment = attachmentNumber;
	attachmentReference.layout = requiredLayout;


}

VkImage& Image::getImage()
{
	return image;
}
VkImageView& Image::getImageView()
{
	return imageView;
}
VkSampler& Image::getSampler()
{
	return sampler;
}

const VkFormat& Image::getFormat()
{
	return format;
}

const VkExtent2D& Image::getExtent()
{
	return extent;
}
