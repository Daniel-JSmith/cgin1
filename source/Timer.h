#pragma once

#include <chrono>

/**
* @brief Container for managing engine time.
* 
* Provides function to check elapsed time per frame
*/
class Timer
{
public:

	/**
	* @brief Updates Timer. Assumes call occurs once every frame.
	* 
	*/
	void update();

	/**
	* @brief Returns the elapsed time in seconds since the last frame.
	* 
	* @return time in seconds since the last frame
	*/
	float getFrameTime() const;

	// returns seconds since epoch from system clock
	float getCurrentTime() const;

private:

	std::chrono::high_resolution_clock::time_point initialTime = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point previousTime = std::chrono::high_resolution_clock::now();
	float deltaTime = 0;
};

