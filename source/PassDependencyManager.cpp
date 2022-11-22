#include "PassDependencyManager.h"

#include <unordered_set>

std::unordered_map<Pass*, std::vector<ResourceAccessHazard>> PassDependencyManager::predecessorResources{};

void PassDependencyManager::preparePasses()
{
	for (const auto& pair : predecessorResources)
	{
		pair.first->prepareExecution(&insertBarriers);
	}
}

void PassDependencyManager::registerPasses(std::vector<DependencyList> dependencies)
{
	predecessorResources.clear();

	for (const DependencyList& dependencyList : dependencies)
	{
		std::vector<ResourceAccessSpecifier> dstAccesses;
		dependencyList.pass->getResources(dstAccesses);

		std::vector<ResourceAccessHazard> sharedAccesses;

		for (const auto& dstAccess : dstAccesses)
		{
			for (const auto& srcPass : dependencyList.dependenices)
			{
				std::vector<ResourceAccessSpecifier> srcAccesses;
				srcPass->getResources(srcAccesses);

				for (const auto& srcAccess : srcAccesses)
				{
					if (srcAccess.resource == dstAccess.resource)
					{
						sharedAccesses.push_back(ResourceAccessHazard{ srcAccess.resource, srcAccess.accessSpecifier, dstAccess.accessSpecifier });
					}
				}
			}
		}

		predecessorResources.insert(std::pair<Pass*, std::vector<ResourceAccessHazard>>(dependencyList.pass, sharedAccesses));
	}

	// record command buffers etc. We have to call this after establishing dependencies because because it creates synchronization logic with the GPU API
	preparePasses();
}


void PassDependencyManager::insertBarriers(VkCommandBuffer commandBuffer, Pass* pass)
{
	std::unordered_set<Resource*> preparedResources;

	for (const ResourceAccessHazard& access : predecessorResources.at(pass))
	{
		access.resource->insertBarrier(commandBuffer, access.srcAccess, access.dstAccess);
		preparedResources.insert(access.resource);
	}

	// other preparation (e.g. memory layout transfer). This code is only used for the initial access of a resource in the pipeline, not accesses between passes.

	std::vector<ResourceAccessSpecifier> accesses;
	pass->getResources(accesses);
	for (const ResourceAccessSpecifier& access : accesses)
	{
		if (preparedResources.find(access.resource) == preparedResources.end())
		// Assume no previous access. pass is the first pass to acces access.resource in the pipeline
		access.resource->insertBarrier(commandBuffer, {AccessSpecifier::OPERATION::NO_OPERATION, AccessSpecifier::STAGE::INITIAL}, access.accessSpecifier);
	}
}
