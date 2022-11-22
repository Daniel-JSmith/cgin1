#include "Camera.h"

#include <iostream>

Camera::Camera(float verticalFov, float aspectRatio) : verticalFov(verticalFov), aspectRatio(aspectRatio)
{
}

void Camera::offsetPosition(const glm::vec3& axis, float delta)
{
	transform.offsetPosition(axis, delta);
}

bool inline pitchTooSteep(glm::vec3 up, glm::vec3 forward, float maxPitch)
{
	return glm::abs(glm::dot(up, forward)) > glm::cos(maxPitch);
}

Transform& Camera::getTransform()
{
	return transform;
}

// not too steep
bool inline pitchWithinBounds(Transform& transform, float maxRadians)
{
	float angle = glm::abs(glm::half_pi<float>() - glm::acos(glm::dot(transform.getForward(), CoordinateSystem::UP)));
	return angle < maxRadians;
}

void Camera::rotate(const glm::vec3& axis, float radians)
{
	Transform test = transform;
	test.rotate(axis, radians);
	if (pitchWithinBounds(test, MAX_VERTICAL_RADIANS))
	{
		transform = test;
	}
}

glm::vec3 Camera::getForward() const
{
	return transform.getForward();
	
}
glm::vec3 Camera::getRight() const
{
	return glm::normalize(glm::cross(getForward(), glm::vec3(0, 1, 0)));
}

glm::mat4 Camera::getVP() const
{
	glm::mat4 P = glm::perspective(glm::radians(verticalFov), aspectRatio, 0.1f, 100.0f);
	// vulkan's and glm's coordinate systems have opposite y-directions
	P[1][1] *= -1;

	glm::mat4 V = glm::lookAt(transform.getPosition(), transform.getPosition() + getForward(), CoordinateSystem::UP);

	return  P * V;
}