#include "GeometryContainer.h"

VkVertexInputBindingDescription GeometryContainer::getBindingDescription() const
{
	return bindingDescription;
}

const std::vector<VkVertexInputAttributeDescription>& GeometryContainer::getAttributeDescriptions() const
{
	return attributeDescriptions;
}

Buffer& GeometryContainer::getVertexBuffer() const
{
	return *vertexBuffer;
}

Buffer& GeometryContainer::getIndexBuffer() const
{
	return *indexBuffer;
}

uint32_t GeometryContainer::getNumIndices() const
{
	return numIndices;
}
VkIndexType GeometryContainer::getIndexType() const
{
	return indexType;
}

VkFrontFace GeometryContainer::getWindingOrder() const
{
	return windingOrder;
}

VkVertexInputBindingDescription GeometryContainer::createBindingDescription(int vertexByteSize)
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = vertexByteSize;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> GeometryContainer::createAttributeDescription(int numAttributes)
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(numAttributes);

	for (int i = 0; i < numAttributes; i++)
	{
		attributeDescriptions[i].binding = 0;
		attributeDescriptions[i].location = i;
		attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[i].offset = i * sizeof(glm::vec3);
	}

	return attributeDescriptions;
}