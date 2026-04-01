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
		Body* hoveredBody;
		Body* selectedBody;
		float currentTime;
		float oldTime;
		float deltaTime;
		float elapsedTime;
		std::vector<Body> system;
		std::vector<Body*> emissives;
		std::vector<float> stars;

		std::optional<Body> editingBody;
		bool positionConfirmed;

	public:
		Renderer();
		int init();
		int initUI();
		void drop();
		void processPhysics();
		void renderBody(Body& body);
		void renderEditingBody();
		void renderStars();
		void processLighting();
		void processRendering();
		void renderNormal();
		void renderEditing();
		void processBloom(int passes);
		void handleCameraMovement(float xOffset, float yOffset);
		void handleCameraZoom(float yOffset);
		void updateFramebufferSize(int fbWidth, int fbHeight);
		void updateWindowSize(int width, int height);
		void selectBody();
		void togglePositionConfirm();
		Body* pickObject(const Camera::Ray& ray);
		GLFWwindow* getWindow();

		enum state {
			NORMAL, EDITING
		};
		state currentState;
};
