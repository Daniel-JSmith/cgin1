#include "Shader.h"
#include "AssetManager.h"

static VkShaderModule getShaderModule(const std::vector<char>& code, VkDevice& device)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module");
	}

	return shaderModule;
}

Shader::Shader(VulkanCore& vulkanCoreSupport, const std::vector<char>& code) : vulkanCoreSupport(vulkanCoreSupport), shaderModule(getShaderModule(code, vulkanCoreSupport.getDevice()))
{
}

Shader::Shader(VulkanCore& vulkanCoreSupport, const std::string& filename) : vulkanCoreSupport(vulkanCoreSupport)
{
	std::vector<char> code = AssetManager::readText(filename);
	shaderModule = getShaderModule(code, vulkanCoreSupport.getDevice());
}

Shader::~Shader()
{
	vkDestroyShaderModule(vulkanCoreSupport.getDevice(), shaderModule, nullptr);
}

const VkShaderModule& Shader::getModule() const
{
	return shaderModule;
}