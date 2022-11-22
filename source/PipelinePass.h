#pragma once

#include "Pass.h"

/**
* @brief Pass that executes user-defined shader code.
*/
class PipelinePass : public Pass
{
public:

	/**
	* @brief Creates a PipelinePass.
	* 
	* @param resources what resources the pass will use and how
	*/
	PipelinePass(VulkanCore& vulkanCoreSupport, std::vector<ResourceShaderInterface> resources);
	virtual ~PipelinePass();
	virtual void prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers) override;

protected:

	/**
	* @brief handle to Vulkan pipeline layout object
	*/
	VkPipelineLayout pipelineLayout;

	/**
	* @brief hanlde to Vulkan pipeline object
	*/
	VkPipeline pipeline;

private:

	virtual void createPipelineLayout();
	virtual void createPipeline() = 0;
};