#pragma once
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Image.h"
#include "VulkanCore.h"
#include "ResourceAccessSpecifier.h"
#include "PipelinePass.h"
#include "Shader.h"
#include "GeometryContainer.h"

/**
* @brief Pass that executes a draw command on a Mesh, using user-supplied vertex and fragment shader code.
*/
class DrawPass : public PipelinePass
{
public:

	/**
	* @brief Creates a DrawPass.
	* 
	* @param resources what resources the pass will use and how
	* @param vertexShaderPath location of vertex shader code
	* @param fragmentShaderPath location of fragment shader code
	* @param mesh geometry to execute draw command on
	*/
	DrawPass(VulkanCore& vulkanCoreSupport, std::vector<ResourceShaderInterface> resources, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const GeometryContainer& mesh);
	~DrawPass();

	void prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers) override;

private:
	const GeometryContainer& mesh;
	Shader vertexShader;
	Shader fragmentShader;
	std::vector<ResourceShaderInterface> outputAttachments;
	VkRenderPass renderPass;
	VkFramebuffer frameBuffer;

	void createPipeline() override;
	void createRenderPass();
	void createFrameBuffer();
	void recordCommandBuffer(std::function<void(VkCommandBuffer, Pass*)> insertBarriers) override;
};