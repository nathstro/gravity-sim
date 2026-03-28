#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.hpp"

int main()
{
	Renderer renderer;
	if (renderer.init())
	{
		return -1;
	}

	GLFWwindow* window = renderer.getWindow();

	while (!glfwWindowShouldClose(window))
	{
		renderer.processRendering();
	}

	renderer.drop();
	return 0;
}