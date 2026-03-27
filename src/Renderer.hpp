#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "models/Sphere.hpp"
#include "Body.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

class Renderer
{
	private:
		GLFWwindow* window;
		Sphere baseSphere;
		unsigned int sphereVAO, sphereVBO, sphereEBO;
		Shader* shader;
		Camera* cam;
		float currentTime;
		float oldTime;
		float deltaTime;
		std::vector<Body> system;

	public:
		Renderer();
		int init();
		void drop();
		void processPhysics();
		void renderBody(Body& body);
		void processRendering();
		void handleCameraMovement(float xOffset, float yOffset);
		GLFWwindow* getWindow();
};
