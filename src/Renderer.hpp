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
		unsigned int trailVAO, trailVBO;
		Shader *sphereShader, *trailShader;
		Camera* cam;
		float currentTime;
		float oldTime;
		float deltaTime;
		float elapsedTime;
		std::vector<Body> system;

	public:
		Renderer();
		int init();
		void drop();
		void processPhysics();
		void renderBody(Body& body, bool withTrail);
		void processRendering();
		void handleCameraMovement(float xOffset, float yOffset);
		GLFWwindow* getWindow();
};
