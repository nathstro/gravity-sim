#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
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
		unsigned int hoveredBody;
		unsigned int selectedBody;
		float currentTime;
		float oldTime;
		float deltaTime;
		float elapsedTime;
		std::vector<std::unique_ptr<Body>> system;
		std::vector<unsigned int> emissives;
		std::vector<float> stars;

		std::optional<Body> editingBody;
		bool positionConfirmed;
		unsigned int nextID;
		Body* getBody(unsigned int ID);

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

		void addBody(std::unique_ptr<Body> body);
		void removeBody(unsigned int bodyID);
		unsigned int pickObject(const Camera::Ray& ray);
		GLFWwindow* getWindow();

		enum state {
			NORMAL, EDITING
		};
		state currentState;
};
