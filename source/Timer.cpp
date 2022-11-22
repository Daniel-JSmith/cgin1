#include "Timer.h"

void Timer::update()
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

	deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();

	previousTime = currentTime;
}

float Timer::getFrameTime() const
{
	return deltaTime;
}

float Timer::getCurrentTime() const
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<float, std::chrono::seconds::period>(currentTime - initialTime).count();
}