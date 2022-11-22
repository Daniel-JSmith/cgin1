#pragma once
#include "PresentationController.h"
#include "PassDependencyManager.h"
#include "memory"

class WorkContainer
{

private:

	VulkanCore& vulkanCoreSupport;
	
	std::unique_ptr<PresentationController> presentationController;

	bool initialized;

	void init(std::vector<DependencyList> passDependencies, std::vector<Resource*>& resources, Image& presentImage);

public:
	WorkContainer(VulkanCore& vulkanCoreSupport);
	WorkContainer(const WorkContainer&) = delete;
	WorkContainer(WorkContainer&&) = delete;

	WorkContainer& operator=(const WorkContainer&) = delete;
	WorkContainer& operator=(WorkContainer&&) = delete;

	void run(std::vector<DependencyList>& passDependencies, std::vector<Resource*>& resources, Image& presentImage);
};