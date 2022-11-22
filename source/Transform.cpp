#include "Transform.h"

Transform::Transform() : position(glm::vec3(0, 0, 0)), orientation(glm::quat(1,0,0,0)), scale(glm::vec3(1, 1, 1))
{

}

Transform::~Transform()
{

}

void Transform::offsetPosition(const glm::vec3& axis, float delta)
{
	position += (axis * delta);
}

void Transform::setPosition(const glm::vec3& newPosition)
{
	position = newPosition;
}

void Transform::rotate(const glm::vec3& axis, float radians)
{
	glm::quat rotation = glm::angleAxis(radians, axis);
	orientation = rotation * orientation;
}

void Transform::getM(glm::mat4x4& M, glm::mat4x4& normal) const
{
	// translation
	glm::mat4x4 T = glm::translate(glm::mat4x4(1), position);

	glm::mat4x4 RS = glm::toMat4(orientation);

	// scale
	RS = glm::scale(RS, scale);

	normal = glm::transpose(glm::inverse(RS));
	M = T * RS;
}


glm::mat4x4 Transform::getM() const
{
	glm::mat4x4 model;
	glm::mat4x4 normal;

	getM(model, normal);

	return model;
}


float Transform::getPitch() const
{
	return glm::pitch(orientation);
}

glm::vec3 Transform::getForward() const
{
	return glm::rotate(orientation, glm::vec4(CoordinateSystem::FORWARD, 1));
}

glm::vec3 Transform::getRight() const
{
	return CoordinateSystem::RIGHT * orientation;
}

glm::vec3 Transform::getPosition() const
{
	return position;
}
