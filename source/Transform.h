#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "CoordinateSystem.h"

/**
* @brief Description of pose and scale.
* 
* Provides functions to adjust position and rotation.
*/
class Transform
{

public:

	Transform();
	~Transform();

	/**
	* @brief Translates this Transform delta units along axis.
	*
	* @param axis axis to translate on
	* @param delta distance to translate
	*/
	void offsetPosition(const glm::vec3& axis, float delta);

	void setPosition(const glm::vec3& newPosition);

	/**
	* @brief Rotates this Transform radians about axis.
	*
	* @param axis axis to rotate about
	* @param radians amount to rotate in radians
	*/
	void rotate(const glm::vec3& axis, float radians);

	/**
	* @brief Returns model matrix.
	* 
	* @return model matrix
	*/
	glm::mat4x4 getM() const;

	/**
	* @brief Returns model and normal matrices.
	* 
	* @param M variable to return model matrix through
	* @param normal variable to return normal matrix through
	*/
	void getM(glm::mat4x4& M, glm::mat4x4& normal) const;

	/**
	* @brief returns pitch in radians.
	* 
	* @return pitch of this Transform
	*/
	float getPitch() const;

	/**
	* @brief Returns forward vector of this Transform.
	*
	* @return forward direction
	*/
	glm::vec3 getForward() const;

	/**
	* @brief Returns right vector of this Transform.
	*
	* @return right direction
	*/
	glm::vec3 getRight() const;

	/**
	* @brief Returns position of this Transform.
	* 
	* @return position
	*/
	glm::vec3 getPosition() const;

private:
	glm::vec3 position;
	glm::vec3 scale;
	glm::quat orientation;
};