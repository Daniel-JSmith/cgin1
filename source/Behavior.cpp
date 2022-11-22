#include "Behavior.h"
#include "CoordinateSystem.h"
#include "InputSupport.h"
#include <cmath>

void Behavior::rotation(Transform& target, const Timer& timer, float rotateDegreesPerSecond, const glm::vec3& axis)
{
    target.rotate(axis, glm::radians(timer.getFrameTime() * rotateDegreesPerSecond));
}

void Behavior::FPSCameraMovement(Camera& target, const Timer& timer, float translateUnitsPerSecond, float rotateDegreesPerPixel)
{
    // mouse input
    double mouseXDifference, mouseYDifference;
    InputSupport::getMouseDelta(mouseXDifference, mouseYDifference);

    target.rotate(target.getRight(), glm::radians(static_cast<float>(mouseYDifference)));
    target.rotate(CoordinateSystem::UP, glm::radians(static_cast<float>(mouseXDifference)));

    // key input

    if (InputSupport::isKeyPressed(GLFW_KEY_A))
    {
        target.offsetPosition(target.getRight(), -translateUnitsPerSecond * timer.getFrameTime());
    }
    if (InputSupport::isKeyPressed(GLFW_KEY_S))
    {
        target.offsetPosition(target.getForward(), -translateUnitsPerSecond * timer.getFrameTime());
    }
    if (InputSupport::isKeyPressed(GLFW_KEY_D))
    {
        target.offsetPosition(target.getRight(), translateUnitsPerSecond * timer.getFrameTime());
    }
    if (InputSupport::isKeyPressed(GLFW_KEY_W))
    {
        target.offsetPosition(target.getForward(), translateUnitsPerSecond * timer.getFrameTime());
    }
}
