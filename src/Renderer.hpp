#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/fwd.hpp"
#include "models/Sphere.hpp"
#include "Body.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

class Renderer
{
	private:
		GLFWwindow* window;
		int width, height, fbWidth, fbHeight;
		Sphere baseSphere;
		unsigned int sphereVAO, sphereVBO, sphereEBO;
		unsigned int starsVAO, starsVBO;
		unsigned int quadVAO, quadVBO;
		unsigned int FBO, RBO, colourBufferTexture, lightBufferTexture;
		unsigned int pingpongFBO[2], pingpongTexture[2];
		Shader *shader, *starShader, *postProcessingShader, *blurShader;
		Camera* cam;
		float currentTime;
		float oldTime;
		float deltaTime;
		float elapsedTime;
		std::vector<Body> system;
		std::vector<Body*> emissives;
		std::vector<float> stars;

	public:
		Renderer();
		int init();
		void drop();
		void processPhysics();
		void renderBody(Body& body);
		void renderStars();
		void processLighting();
		void processRendering();
		void processBloom(int passes);
		void handleCameraMovement(float xOffset, float yOffset);
		void updateFramebufferSize(int fbWidth, int fbHeight);
		void updateWindowSize(int width, int height);
		GLFWwindow* getWindow();
};
