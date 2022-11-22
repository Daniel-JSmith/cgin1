#pragma once

#include <string>

#include "VulkanCore.h"

/**
* @brief Wrapper for a shader file and the associated shader object in the GPU API.
*/
class Shader
{
public:

	/**
	* @brief Creates a Shader object from shader code.
	* 
	* @param code SPIRV code
	*/
	Shader(VulkanCore& vulkanCoreSupport, const std::vector<char>& code);

	/**
	* @brief Creates a Shader object from code in a file.
	* 
	* @param filename location of shader code to read
	*/
	Shader(VulkanCore& vulkanCoreSupport, const std::string& filename);

	~Shader();

	/**
	* @brief Returns underlying Vulkan shader module object.
	* 
	* @return Vulkan shader module object
	*/
	const VkShaderModule& getModule() const;

private:

	VulkanCore& vulkanCoreSupport;

	VkShaderModule shaderModule;
};