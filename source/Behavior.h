#pragma once
#include "Transform.h"
#include "Timer.h"
#include "Camera.h"


namespace Behavior
{

    void rotation(Transform& target, const Timer& timer, float rotateDegreesPerSecond, const glm::vec3& axis);

    void FPSCameraMovement(Camera& target, const Timer& timer, float translateUnitsPerSecond, float rotateDegreesPerPixel);

}
