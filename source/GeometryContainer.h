#pragma once

#include <vector>
#include <memory>
#include "Buffer.h"
#include <glm/glm.hpp>


/**
* @brief Container for a vertex buffer. Essentially a mesh.
*/
class GeometryContainer
{
public:

	/**
	* @brief Creates a new GeometryContainer with 16 bit indices.
	* 
	* @param vertexData pointer to vertex data
	* @param vertexDataSize size of vertex data in bytes
	* @param numVertices number of vertexes in vertexData
	* @param indices pointer to index data
	* @param numIndices number of indices
	* @param windingOrder vertex winding order
	*/
	template <typename T>
	GeometryContainer(VulkanCore& vulkanCoreSupport, const std::vector<T>& vertices, const std::vector<uint16_t>& indices, VkFrontFace windingOrder) : windingOrder(windingOrder)
	{
		init(vulkanCoreSupport, vertices, indices);
		indexType = VK_INDEX_TYPE_UINT16;
	}

	/**
	* @brief Creates a new GeometryContainer with 32 bit indices.
	*
	* @param vertexData pointer to vertex data
	* @param vertexDataSize size of vertex data in bytes
	* @param numVertices number of vertexes in vertexData
	* @param indices pointer to index data
	* @param numIndices number of indices
	* @param windingOrder vertex winding order
	*/
	template <typename T>
	GeometryContainer(VulkanCore& vulkanCoreSupport, const std::vector<T>& vertices, const std::vector<uint32_t>& indices, VkFrontFace windingOrder) : windingOrder(windingOrder)
	{
		init(vulkanCoreSupport, vertices, indices);
		indexType = VK_INDEX_TYPE_UINT32;
	}

	/**
	* @brief Returns Vulkan binding description.
	* 
	* @return Vulkan binding description
	*/
	VkVertexInputBindingDescription getBindingDescription() const;

	/**
	* @brief Returns Vulkan attribute descriptions.
	* 
	* @return Vulkan attribute descriptions
	*/
	const std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions() const;

	/**
	* @brief Returns vertex buffer.
	* 
	* @return vertex buffer
	*/
	Buffer& getVertexBuffer() const;

	/**
	* @brief Returns index buffer.
	* 
	* @return index buffer
	*/
	Buffer& getIndexBuffer() const;

	/**
	* @brief Returns number of indices.
	* 
	* @return number of indices
	*/
	uint32_t getNumIndices() const;

	/**
	* @brief Returns type of indices.
	* 
	* @return index type
	*/
	VkIndexType getIndexType() const;

	/**
	* @brief Returns vertex winding order.
	* 
	* @return winding order
	*/
	VkFrontFace getWindingOrder() const;

private:
	VkFrontFace windingOrder;
	VkIndexType indexType;

	uint32_t numIndices;

	std::unique_ptr<Buffer> vertexBuffer;
	std::unique_ptr<Buffer> indexBuffer;

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	static VkVertexInputBindingDescription createBindingDescription(int vertexByteSize);

	static std::vector<VkVertexInputAttributeDescription> createAttributeDescription(int numAttributes);

	using attributeType = glm::vec3;

	template <typename VertexType, typename IndexType>
	void init(VulkanCore& vulkanCoreSupport, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices)
	{
		VkDeviceSize vertexBufferSize = sizeof(VertexType) * vertices.size();
		vertexBuffer = std::make_unique<Buffer>(vulkanCoreSupport, vertexBufferSize, AccessSpecifier::OPERATION::VERTEX_BUFFER, Resource::ACCESS_PROPERTY::GPU_PREFERRED, vertices.data());

		VkDeviceSize indexBufferSize = sizeof(IndexType) * indices.size();
		indexBuffer = std::make_unique<Buffer>(vulkanCoreSupport, indexBufferSize, AccessSpecifier::OPERATION::INDEX_BUFFER, Resource::ACCESS_PROPERTY::GPU_PREFERRED, indices.data());

		bindingDescription = createBindingDescription(static_cast<int>(vertexBufferSize / vertices.size()));
		attributeDescriptions = createAttributeDescription(static_cast<int>(vertexBufferSize / vertices.size() / sizeof(attributeType)));
		numIndices = static_cast<uint32_t>(indices.size());
	}
};