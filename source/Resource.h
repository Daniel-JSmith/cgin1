#pragma once
#include <set>
#include <functional>
#include "VulkanCore.h"
#include "AccessSpecifier.h"

/**
* @brief Representation of GPU-accessible memory. Provides functions wrapping GPU API functions: resource initialization, synchronization, etc.
* 
* GPU APIs usually distinguish between buffers and images. Generally, buffers may contain arbitrary data while images may be used with relevant built-in operations such as sampling etc. Here, Buffer and Image are separate classes inheriting from Resource.
* 
* @see Buffer
* @see Image
*/
class Resource
{
public:

	/**
	* @brief Indication of how memory should be transfered to GPU.
	*/
	enum ACCESS_PROPERTY
	{
		/// Preference for memory allowing quick CPU access
		CPU_PREFERRED,
		/// Preference for memory allowing quick GPU access
		GPU_PREFERRED,
	};

	Resource(VulkanCore& vulkanCoreSupport);

	/**
	* @brief Destructor
	*/
	virtual ~Resource() = default;

	/**
	* @brief Creates a VkDescriptorSetLayoutBinding struct representing this Descriptor
	* 
	* @param index binding of the descriptor
	* @param access description of how descriptor is accessed
	* @param out result
	*/
	virtual void createLayoutBinding(int index, AccessSpecifier access, VkDescriptorSetLayoutBinding& out) const = 0;

	/**
	* @brief Creates a VkWriteDescriptorSet struct representing this Descriptor
	* 
	* @param index binding of the descriptor
	* @param access description of how descriptor is accessed
	* @param descriptorSet VkDescriptorSet to record write to
	* @param out result
	*/
	virtual void createDescriptorWrite(int index, AccessSpecifier access, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const = 0;


	/**
	* @brief Prepares this Resource for the access decribed in currentAccess. Equivalent to prepareForAccess where previousAccess represents an initial access
	* 
	* @param commandBuffer command buffer to record to
	* @param currentAccess how this Resource is accessed
	*/
	virtual void prepareForInitialAccess(VkCommandBuffer& commandBuffer, AccessSpecifier currentAccess) = 0;

	/**
	* @brief Inserts a barrier to commandBuffer
	*
	* @param commandBuffer command buffer to record to
	* @param previousAccess how this Resource is accessed before this access
	* @param currentAccess how this Resource is accessed
	*/
	virtual void insertBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier previousAccess, AccessSpecifier currentAccess) = 0;

	/**
	* @brief Notifies this Resource that it is used as in accessSpecifier. Ensures that the host will not update this resource while used by GPU.
	* 
	* @param passFence fence signaling whether a pass is running
	* @param accessSpecifier how this Resource is accessed.
	*/
	void registerResourceUse(const VkFence& passFence, AccessSpecifier accessSpecifier);

	/**
	* @brief Prepares this Resource for use. Must be called after all passes have registered and before execution begins.
	* 
	*/
	void initialize();

protected:

	/// Maps internal enum to vulkan enum
	static const std::unordered_map<ACCESS_PROPERTY, VkMemoryPropertyFlags> MEMORY_PROPERTY_FLAGS;
	/// Maps internal enum to vulkan enum
	static const std::unordered_map<AccessSpecifier::OPERATION, VkDescriptorType> DESCRIPTOR_TYPES;
	/// Maps internal enum to vulkan enum
	static const std::unordered_map<AccessSpecifier::STAGE, VkShaderStageFlagBits> STAGE_FLAGS;

	/// Maps internal enum to vulkan enum
	static const std::unordered_map<AccessSpecifier::OPERATION, VkAccessFlags> ACCESS_FLAGS;

	/// Maps internal enum to vulkan enum
	static const std::unordered_map<AccessSpecifier::STAGE, VkPipelineStageFlagBits> PIPELINE_STAGE_FLAGS;

	

	/**
	* @brief Maps an AccessSpecifer to a Vulkan access flag.
	* 
	* @param accessSpecifer struct to get access flag from
	* 
	* @return Vulkan access flag from an AccessSpecifier
	*/
	VkAccessFlags getAccessFlag(const AccessSpecifier& accessSpecifier);

	/**
	* @brief Maps an AccessSpecifer to a Vulkan pipeline stage flag.
	* 
	* @param accessSpecifer struct to get pipeline stage flag from
	*
	* @return Vulkan pipeline stage flag from an AccessSpecifier
	*/
	VkPipelineStageFlagBits getPipelineStageFlag(const AccessSpecifier& accessSpecifier);

	/**
	* @brief blocks until this Resource is not in use by GPU.
	*/
	virtual void waitForReady() const;

	/**
	* @brief Operations the user has declared to use on this Resource.
	*/
	std::set<AccessSpecifier::OPERATION> accessTypes;

	/**
	* @brief Function to initialize this Resource with.
	*/
	std::function<void()> initializeFunction;

	VulkanCore& getVulkanCoreSupport() const;

private:

	VulkanCore& vulkanCoreSupport;

	// each elements represents whether a command buffer that uses this Resource is not in the queue. Used to synchronize host writes
	std::vector<VkFence> notInUseFences;
};
