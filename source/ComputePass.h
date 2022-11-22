#pragma once

#include "PipelinePass.h"
#include "Shader.h"

/**
* @brief Pass that executes a user-defined compute shader.
*/
class ComputePass : public PipelinePass
{

public:

	/**
	* @brief Creates a ComputePass.
	* 
	* @param resources what resources the pass will use and how
	* @param computeShaderPath location of shader code
	*/
	ComputePass(VulkanCore& vulkanCoreSupport, std::vector<ResourceShaderInterface> resources, const std::string& computeShaderPath);

	~ComputePass();

	void prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers) override;

private:
	Shader computeShader;

	void createPipeline() override;

	void recordCommandBuffer(std::function<void(VkCommandBuffer, Pass*)> insertBarriers);
};