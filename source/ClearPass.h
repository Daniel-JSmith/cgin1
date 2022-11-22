#include "Pass.h"

/**
* @brief Pass that clears an Image.
* 
* Executing a ClearPass clears the associated image.
*/
class ClearPass : public Pass
{
public:

	/**
	* @brief Creates a ClearPass.
	* 
	* @param image image ClearPass will operate on
	*/
	ClearPass(VulkanCore& vulkanCoreSupport, Image& image);

	void prepareExecution(std::function<void(VkCommandBuffer, Pass*)> insertBarriers) override;

private:
	Image& image;

	void recordCommandBuffer(std::function<void(VkCommandBuffer, Pass*)> insertBarriers) override;
};