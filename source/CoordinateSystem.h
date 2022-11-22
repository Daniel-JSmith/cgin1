#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace CoordinateSystem
{
	const glm::vec3 UP(0, 1, 0);
	const glm::vec3 RIGHT(1, 0, 0);
	const glm::vec3 FORWARD(0, 0, 1);
}