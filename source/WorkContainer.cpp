#include "WorkContainer.h"
#include "PresentationController.h"
#include "PassDependencyManager.h"
#include "Timer.h"

std::vector<Pass*> dependencyListToVector(const std::vector<DependencyList>& dependencyLists)
{
	std::vector<Pass*> result;
	for (const DependencyList& dependencyList : dependencyLists)
	{
		result.push_back(dependencyList.pass);
	}

	return result;
}


void WorkContainer::init(std::vector<DependencyList> passDependencies, std::vector<Resource*>& resources, Image& presentImage)
{
	presentationController = std::make_unique<decltype(presentationController)::element_type>(vulkanCoreSupport, presentImage);

	for (const auto& resource : resources)
	{
		resource->initialize();
	}

	Pass* finalUserPass = passDependencies.at(passDependencies.size() - 1).pass;

	std::vector<Pass*> presentPasses;
	presentationController->getPasses(presentPasses);
	for (const auto& pass : presentPasses)
	{
		passDependencies.push_back(DependencyList{ pass, { finalUserPass } });
	}

	auto passes = dependencyListToVector(passDependencies);
	std::vector<VkDescriptorType> descriptorTypes;
	for (Pass* const & pass : passes)
	{
		std::vector<ResourceAccessSpecifier> accesses;
		pass->getResources(accesses);
		for (const ResourceAccessSpecifier& resourceAccessSpecifier : accesses)
		{
			AccessSpecifier::OPERATION operation = resourceAccessSpecifier.accessSpecifier.operation;

			if (VulkanCore::descriptorTypes.count(operation) > 0) // An operation is a descriptor if it exists in descriptorTypes
			{
				descriptorTypes.push_back(VulkanCore::descriptorTypes[operation]);
			}
		}
	}

	vulkanCoreSupport.createDescriptorPool(descriptorTypes, static_cast<int>(passes.size()));

	PassDependencyManager::registerPasses(passDependencies);
}



WorkContainer::WorkContainer(VulkanCore& vulkanCoreSupport) : vulkanCoreSupport(vulkanCoreSupport), initialized(false)
{
}

void WorkContainer::run(std::vector<DependencyList>& passDependencies, std::vector<Resource*>& resources, Image& presentImage)
{
	if (!initialized)
	{
		init(passDependencies, resources, presentImage);
		initialized = true;
	}

	for (const auto& dependency : passDependencies)
	{
		dependency.pass->execute();
	}

	presentationController->present();
}
