#pragma once
#include <iostream>
#include <GLFW/glfw3.h>

/**
* @brief Container for managing user input.
* 
* Provides functions to check mouse and keyboard state.
*/
class InputSupport
{
public:

	/**
	* @brief initializes cursor and registers GLFW callbacks.
	* 
	* @param window handle to indow capturing input
	*/
	static void initialize(GLFWwindow* window);

	/**
	* @brief Returns whether a key is clicked.
	* 
	* A key is clicked on only the first frame the user presses the key.
	* 
	* @param keyCode key to check
	* 
	* @return whether key represented by keyCode is clicked
	*/
	static bool isKeyClicked(int keyCode);

	/**
	* @brief Returns whether a key is pressed.
	* 
	* A key is pressed at any time it is down, not only on a certain frame.
	* 
	* @param keyCode key to check
	* 
	* @return whether key represented by keyCode is pressed
	*/
	static bool isKeyPressed(int keyCode);

	/**
	* @brief Returns distance the cusor has moved since the last frame.
	* 
	* @param dX variable to fill with x distance moved in pixels
	* @param dY variable to fill with y distance moved in pixels
	*/
	static void getMouseDelta(double& dX, double& dY);

private:
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
};