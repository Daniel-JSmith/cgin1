#pragma once

#include "Transform.h"

/**
* @brief A simple camera. Provides functions for transforming the camera and retrieving the view matrix.
*/
class Camera
{
private:

	static constexpr float SENSITIVITY = 0.5f;

	// greatest pitch allowed
	static constexpr float MAX_VERTICAL_DEGREES = 85.0f;
	static constexpr float MAX_VERTICAL_RADIANS = glm::radians(MAX_VERTICAL_DEGREES);
	static constexpr float MAX_PITCH = glm::half_pi<float>() - glm::radians(MAX_VERTICAL_DEGREES);

	float verticalFov;
	float aspectRatio;

	Transform transform;

public:

	/**
	* @brief Creates a camera according to arguments.
	* 
	* @param verticalFov vertical field of view of the camera
	* @param aspectRatio width/height ratio of the camera
	*/
	Camera(float verticalFov, float aspectRatio);

	/**
	* @brief Translates this Camera delta units along axis.
	* 
	* @param axis axis to translate on
	* @param delta distance to translate
	*/
	void offsetPosition(const glm::vec3& axis, float delta);

	/**
	* @brief Rotates this Camera radians about axis.
	* 
	* @param axis axis to rotate about
	* @param radians amount to rotate in radians
	*/
	void rotate(const glm::vec3& axis, float radians);

	Transform& getTransform();

	/**
	* @brief Returns forward vector of this Camera.
	* 
	* @return Direction this Camera faces
	*/
	glm::vec3 getForward() const;

	/**
	* @brief Returns right vector of this Camera.
	* 
	* @return Vector pointing to the camera's right
	*/
	glm::vec3 getRight() const;

	/**
	* @brief Returns view-projection matrix of this Camera.
	* 
	* @return view-projection matrix
	*/
	glm::mat4 getVP() const;

};