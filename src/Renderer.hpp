#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "model/Sphere.hpp"
#include "Body.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

class Renderer
{
	private:
		GLFWwindow* window;
		Sphere baseSphere;
		unsigned int sphereVAO, sphereVBO, sphereEBO;
		Shader shader;
		Camera cam;
		float currentTime;
		float oldTime;
		float deltaTime;
		std::vector<Body> system;

	public:
		Renderer();
		int init();
		void drop();
		// void processPhysics(Body body);
		void renderBody(Body body);
		void processRendering();
		GLFWwindow* getWindow();
};
