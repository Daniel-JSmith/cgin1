#pragma once

#include "Pass.h"
#include "PresentationController.h"

/**
* @brief Specification of a Pass object's dependecies on other Passes.
*/
struct DependencyList
{
	/**
	* @brief pass whose dependencies are described in this DependencyList
	*/
	Pass* pass;

	/**
	* @brief passes that pass depends on
	*/
	std::vector<Pass*> dependenices;
};

/**
* @brief Description of how two separate Pass objects access a single Resource.
*/
struct ResourceAccessHazard
{
	/**
	* @brief resource being accessed
	*/
	Resource* resource;

	/**
	* @brief how the first pass accesses the resource
	*/
	AccessSpecifier srcAccess;

	/**
	* @brief how the second pass accesses the resource
	*/
	AccessSpecifier dstAccess;
};

/**
* @brief Container for managing Pass synchronization.
* 
* Only used internally.
*/
class PassDependencyManager
{
public:
	// expects predecessors in a possible execution order. The first element in predecessors represents the first executed pass, the final element the final executed pass

	/**
	* @brief Registers passes with this PassDependencyManager, performing initializing required for execution.
	* 
	* @param dependencies passes to register
	*/
	static void registerPasses(std::vector<DependencyList> dependencies);

private:
	static std::unordered_map<Pass*, std::vector<ResourceAccessHazard>> predecessorResources;

	static void insertBarriers(VkCommandBuffer commandBuffer, Pass* pass);
	static void preparePasses();
};