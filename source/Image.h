#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>

#include "VulkanCore.h"
#include "Buffer.h"
#include "Resource.h"
#include "ResourceAccessSpecifier.h"


/**
* @brief Representation of a GPU-accessible image.
*/
class Image : public Resource
{
public:

	/**
	* @brief Creates an empty image.
	* 
	* @param format format of image
	* @param extent dimensions of image
	* @param accessProperty memory location preference
	*/
	Image(VulkanCore& vulkanCoreSupport, VkFormat format, VkExtent2D extent, ACCESS_PROPERTY accessProperty);

	/**
	* @brief Creates an Image referencing and existing Vulkan image handle.
	* 
	* An Image created this way will not free the passed image handle when deleted
	* 
	* @param image Vulkan image object to refer to
	* @param format format of image
	* @param extent dimensions of image
	*/
	Image(VulkanCore& vulkanCoreSupport, VkImage& image, VkFormat format, VkExtent2D extent);

	/**
	* @brief Creates an Image filled with texture data at path
	* 
	* @param format format of image
	* @param path location of texture data to read
	* @param accessProperty memory location preference
	*/
	Image(VulkanCore& vulkanCoreSupport, VkFormat format, const std::string& path, ACCESS_PROPERTY accessProperty);

	~Image();

	/**
	* @brief Copies buffer to this Image.
	* 
	* Does not resize of transform data in buffer
	* 
	* @param buffer Buffer to copy to this Image
	*/
	void copyBufferToImage(const Buffer& buffer);

	void createLayoutBinding(int index, AccessSpecifier access, VkDescriptorSetLayoutBinding& out) const override;
	void createDescriptorWrite(int index, AccessSpecifier access, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const override;

	void prepareForInitialAccess(VkCommandBuffer& commandBuffer, AccessSpecifier currentAccess) override;
	void insertBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier previousAccess, AccessSpecifier currentAccess) override;

	/**
	* @brief Returns attachment description and attachment reference for this Image, transitioning into the layout required by accessSpecifier.
	* 
	* Assumes layout transitions will be requested (get__Transition()) in the order they appear in the command buffer
	* 
	* @param currentAccess how this Image is accessed
	* @param attachmentNumber the index of the attachment
	* @param attachmentDescription result
	* @param attachmentReference result
	* @param blend whether blending is enabled for this Image
	*/
	void getAttachmentDescription(AccessSpecifier currentAccess, uint32_t attachmentNumber, VkAttachmentDescription& attachmentDescription, VkAttachmentReference& attachmentReference, bool blend);

	/**
	* @brief Returns underlying Vulkan image handle.
	* 
	* @return handle to Vulkan image object
	*/
	VkImage& getImage();

	/**
	* @brief Returns underlying Vulkan image view.
	* 
	* @return handle to Vulkan image view object
	*/
	VkImageView& getImageView();

	/**
	* @brief Returns underlying Vulkan image sampler.
	* 
	* @return handle to Vulkan image sampler object
	*/
	VkSampler& getSampler();

	/**
	* @brief Returns the format of this Image.
	* 
	* @return image format
	*/
	const VkFormat& getFormat();

	/**
	* @brief Returns the dimensions of this Image.
	* 
	* @return image extent
	*/
	const VkExtent2D& getExtent();

private:

	static const std::unordered_map<AccessSpecifier::OPERATION, VkImageLayout> REQUIRED_LAYOUTS;
	static const std::unordered_map<AccessSpecifier::OPERATION, VkBufferUsageFlags> USE_USAGE_FLAGS;
	static const std::unordered_map<AccessSpecifier::OPERATION, VkImageAspectFlags> USE_ASPECT_FLAGS;

	VkImage image;
	VmaAllocation imageAllocation;
	VkImageView imageView;
	VkSampler sampler;
	VkFormat format;
	VkMemoryPropertyFlags memoryProperties;
	VkExtent2D extent;

	// Some Image objects reference an image created elswhere
	bool responsibleForImageDestruction = false;

	void init(VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkExtent2D extent, VkImageAspectFlags aspect);

	void initializeEmptyImage(VkExtent2D extent, VkFormat format, ACCESS_PROPERTY accessProperty);
};

