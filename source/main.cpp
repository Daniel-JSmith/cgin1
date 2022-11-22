#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "WorkContainer.h"
#include "GeometryContainer.h"
#include "DrawPass.h"
#include "ComputePass.h"
#include "ClearPass.h"
#include "Transform.h"
#include "Timer.h"
#include "InputSupport.h"
#include "Behavior.h"

struct UBO
{
	glm::mat4 previousMVP;
	glm::mat4 currentMVP;
	glm::mat4 normalMatrix;
	glm::vec4 screenResolution;
};

struct ExampleVertex
{
	glm::vec3 position;
	glm::vec3 normal;
};

void readModel(std::string filename, std::vector<ExampleVertex>& vertices, std::vector<uint32_t>& indices)
{
	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning, error;

	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, filename.c_str(), "Assets/Scenes/"))
	{
		throw std::runtime_error(warning + error);
	}

	for (const auto& shape : shapes)
	{
		size_t indexOffset = 0;
		for (size_t face = 0; face < shape.mesh.num_face_vertices.size(); face++)
		{
			for (size_t vertex = 0; vertex < shape.mesh.num_face_vertices[face]; vertex++)
			{
				tinyobj::index_t idx = shape.mesh.indices[indexOffset + vertex];

				// vertex position
				tinyobj::real_t vx = attributes.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attributes.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attributes.vertices[3 * size_t(idx.vertex_index) + 2];

				// vertex normal
				tinyobj::real_t nx = attributes.normals[3 * size_t(idx.normal_index) + 0];
				tinyobj::real_t ny = attributes.normals[3 * size_t(idx.normal_index) + 1];
				tinyobj::real_t nz = attributes.normals[3 * size_t(idx.normal_index) + 2];

				ExampleVertex dst{ glm::vec3(vx, vy, vz), glm::vec3(nx, ny, nz) };
				vertices.push_back(dst);

				indices.push_back(static_cast<uint32_t>(indexOffset + vertex));
			}



			indexOffset += shape.mesh.num_face_vertices[face];
		}
	}
}

int main()
{
	std::vector<ExampleVertex> vertices;
	std::vector<uint32_t> indices;
	readModel("Assets/Meshes/mon.obj", vertices, indices);

	VkExtent2D resolution = { 1920 / 2, 1080 / 2 };

	std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor"
	};

	std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VulkanCore vulkanCore(resolution, resolution, "vt", validationLayers, deviceExtensions);

	WorkContainer workContainer(vulkanCore);

	auto mesh = GeometryContainer(vulkanCore, vertices, indices, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	auto ubo = Buffer(vulkanCore, sizeof(UBO), AccessSpecifier::OPERATION::UNIFORM_BUFFER, Resource::ACCESS_PROPERTY::CPU_PREFERRED);
	auto rasterOutput = Image(vulkanCore, VK_FORMAT_R32G32B32A32_SFLOAT, resolution, Resource::ACCESS_PROPERTY::GPU_PREFERRED);
	auto depthBuffer = Image(vulkanCore, VK_FORMAT_D32_SFLOAT, resolution, Resource::ACCESS_PROPERTY::GPU_PREFERRED);
	auto velocityBuffer = Image(vulkanCore, VK_FORMAT_R32G32B32A32_SFLOAT, resolution, Resource::ACCESS_PROPERTY::GPU_PREFERRED);
	auto finalOutput = Image(vulkanCore, VK_FORMAT_R32G32B32A32_SFLOAT, resolution, Resource::ACCESS_PROPERTY::GPU_PREFERRED);

	std::vector<ResourceShaderInterface> resources =
	{
		ResourceShaderInterface{ResourceAccessSpecifier{&rasterOutput, {AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT, AccessSpecifier::STAGE::FRAGMENT_SHADER}}, -1, false},
		ResourceShaderInterface{ResourceAccessSpecifier{&velocityBuffer, {AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT, AccessSpecifier::STAGE::FRAGMENT_SHADER}}, -1, false},
		ResourceShaderInterface{ResourceAccessSpecifier{&depthBuffer, {AccessSpecifier::OPERATION::DEPTH_BUFFER, AccessSpecifier::STAGE::FRAGMENT_SHADER}}, -1, false},
		ResourceShaderInterface{ResourceAccessSpecifier{&ubo, {AccessSpecifier::OPERATION::UNIFORM_BUFFER, AccessSpecifier::STAGE::VERTEX_SHADER}}, 0, false}
	};

	auto samplePass = DrawPass(vulkanCore, resources, "Assets/Shaders/motion.vert.spv", "Assets/Shaders/motion.frag.spv", mesh);

	resources =
	{
		ResourceShaderInterface{ResourceAccessSpecifier{&rasterOutput, {AccessSpecifier::OPERATION::SHADER_STORAGE_IMAGE, AccessSpecifier::STAGE::COMPUTE_SHADER}}, 0, false},
		ResourceShaderInterface{ResourceAccessSpecifier{&velocityBuffer, {AccessSpecifier::OPERATION::SHADER_STORAGE_IMAGE, AccessSpecifier::STAGE::COMPUTE_SHADER}}, 1, false},
		ResourceShaderInterface{ResourceAccessSpecifier{&finalOutput, {AccessSpecifier::OPERATION::SHADER_STORAGE_IMAGE, AccessSpecifier::STAGE::COMPUTE_SHADER}}, 2, false}
	};

	auto blendPass = ComputePass(vulkanCore, resources, "Assets/Shaders/motion.comp.spv");

	std::vector<Resource* >usedResources = { &mesh.getIndexBuffer(), &mesh.getVertexBuffer(), &velocityBuffer, &ubo, &rasterOutput, &finalOutput, &depthBuffer };

	std::vector<DependencyList> predecessors;
	predecessors.push_back(DependencyList{ &samplePass, {} });
	predecessors.push_back(DependencyList{ &blendPass, {&samplePass} });

	Transform objectTransform;

	auto camera = Camera(60, 16 / 9.0f);
	camera.rotate(CoordinateSystem::UP, glm::pi<float>());
	camera.offsetPosition(CoordinateSystem::FORWARD, 5);
	
	Timer timer;

	auto previousMVP = glm::mat4();

	while (vulkanCore.engineRunning())
	{
		// submit gpu commands
		workContainer.run(predecessors, usedResources, finalOutput);
		glfwPollEvents();

		// update scene
		timer.update();
		objectTransform.setPosition(glm::vec3(sin(timer.getCurrentTime() * 2.5f) * 3, 0, 0));
		Behavior::FPSCameraMovement(camera, timer, 2, 1);

		// update UBO
		UBO tempUBO = {};
		glm::mat4x4 M, normalMat;
		objectTransform.getM(M, normalMat);
		tempUBO.previousMVP = previousMVP;
		tempUBO.currentMVP = camera.getVP() * M;
		tempUBO.normalMatrix = normalMat;
		tempUBO.screenResolution.x = static_cast<float>(resolution.width);
		tempUBO.screenResolution.y = static_cast<float>(resolution.height);
		ubo.copyData(sizeof(tempUBO), &tempUBO);

		previousMVP = tempUBO.currentMVP;
	}

	return EXIT_SUCCESS;
}
