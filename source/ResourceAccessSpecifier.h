#pragma once
#include "AccessSpecifier.h"
#include "Resource.h"

/**
* @brief Description of how a resource is accessed.
*/
struct ResourceAccessSpecifier
{
	/**
	* @brief resource being accessed
	*/
	Resource* resource;

	/**
	* @brief how the resource is accessed
	*/
	AccessSpecifier accessSpecifier;
};

/**
* @brief Description of how a resource is accessed from a shader.
*/
struct ResourceShaderInterface
{
	/**
	* @brief how the resource is accessed
	*/
	ResourceAccessSpecifier resource;

	/**
	* @brief descriptor binding index
	* 
	* non-descriptor resources have a binding index of -1.
	*/
	int descriptorBinding = -1;

	/**
	* @brief whether blending is enabled
	*/
	bool blendEnabled = false;

	/**
	* @brief Returns whether described resource is a descriptor.
	* 
	* @return true if the resource is a descriptor
	*/
	bool isDescriptor() const
	{
		return descriptorBinding >= 0;
	}
};
