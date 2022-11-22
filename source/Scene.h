#pragma once

#include "Image.h"
#include "Pass.h"
#include "PassDependencyManager.h"
#include "Resource.h"

/**
* @brief Interface between client code and the core rendering engine.
* 
* Intended to be extended by client-defined classes. Engine core accesses Scene information through extened functions.
*/ 
class Scene
{
public:
	/**
	* @brief Performs preparation required to use the scene.
	* 
	* Called by the core rendering engine before the first frame.
	*/
	virtual void initialize() = 0;

	/**
	* @brief Returns the Image to be presented.
	* 
	* Called by the core rendering engine before first frame. 
	* 
	* @return Image to display
	*/
	virtual Image& getPresentedImage() const = 0;

	/**
	* @brief Returns DependencyList structs that represent the dependency graph of Pass objects in this Scene.
	* 
	* Called by the core rendering engine before first frame. 
	* 
	* @return Passes and their dependencies used in this Scene
	*/
	virtual std::vector<DependencyList> getDependencies() const = 0;

	/**
	* @brief Returns Resource objects used in this scene.
	* 
	* Called by the core rendering engine before first frame. 
	* 
	* @return Resource objects accessed by Pass objects in this Scene
	*/
	virtual std::vector<Resource*> getResources() const = 0;

	/**
	* @brief Updates scene state.
	* 
	* Called by the core rendering engine every frame.
	*/
	virtual void update() = 0;

	/**
	* @brief Runs passes and stores output in the Image returned by getPresentedImage.
	* 
	* Called by the core rendering engine every frame.
	*/
	virtual void run() = 0;

private:

};