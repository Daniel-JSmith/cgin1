#pragma once

/**
* @brief Container describing how a resource is accessed on the GPU
* 
* A representation of which operation on which stage a resource is accessed on the GPU.
*/
struct AccessSpecifier
{
public:

	/**
	* @brief Operations applied to a resource
	*/
	// greater values indicate write operations. Lower values indicate read operations
	enum OPERATION
	{
		// readonly section
		NO_OPERATION,
		COLOR_SAMPLER,
		DEPTH_SAMPLER,
		PREPARE_FOR_PRESENTATION,
		PRESENT,
		TRANSFER_SOURCE,
		VERTEX_BUFFER,
		INDEX_BUFFER,
		UNIFORM_BUFFER,

		// write operations
		COLOR_ATTACHMENT_OUTPUT,
		TRANSFER_DESTINATION,
		SHADER_STORAGE_BUFFER,
		DEPTH_BUFFER,
		SHADER_STORAGE_IMAGE,
		CLEAR_OUTSIDE_RENDER_PASS
	};

	/**
	* @brief shader stages in which an operation can be applied to a resource
	*/
	enum STAGE
	{
		INITIAL,
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		COMPUTE_SHADER,
		TRANSFER,
		FINAL
	};

	/**
	* @brief Returns whether the described operation includes write access.
	* Returns true if the operation described by this AccessSpecifer writes to the resource
	* 
	* @return Whether the operation of this AccessSpecifer includes a write access
	*/
	bool inline isWriteAccess() const
	{
		return operation >= writeSection;
	}

	AccessSpecifier(OPERATION operation, STAGE stage) : operation(operation), stage(stage)
	{

	}


	/**
	* @brief Which operation the specified resource is used for.
	*/
	OPERATION operation;

	/**
	* @brief Which stage the specified resource is used in.
	*/
	STAGE stage;

private:

	const static int writeSection = static_cast<int>(OPERATION::COLOR_ATTACHMENT_OUTPUT);

};
