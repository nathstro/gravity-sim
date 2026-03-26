#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.hpp"

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main(int argc, char const *argv[])
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
		processInput(window, 0.1);
		glfwPollEvents();
	}

	renderer.drop();
	return 0;
}