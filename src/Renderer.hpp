#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Body.hpp"

class Renderer
{
	private:
		GLFWwindow* window;

	public:
		Renderer();
		int init();
		void drop();
		// void processPhysics(Body body);
		void renderBody(Body body);
		void processRendering(std::vector<Body>& toRender);
		GLFWwindow* getWindow();
};
