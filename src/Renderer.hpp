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
		unsigned int pathVAO, pathVBO;
		unsigned int quadVAO, quadVBO;
		unsigned int FBO, RBO, colourBufferTexture, lightBufferTexture;
		unsigned int pingpongFBO[2], pingpongTexture[2];
		Shader *shader, *starShader, *postProcessingShader, *blurShader, *pathShader;
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
		std::vector<glm::vec3> predictedPath;
		unsigned int nextID;
		Body* getBody(unsigned int ID);

	public:
		Renderer();
		int init();
		int initUI();
		void drop();
		void processPhysics();
		void processLighting();
		void processPath();

		void renderBody(Body& body);
		void renderEditingBody();
		void renderStars();
		void renderPath();

		void processRendering();
		void renderNormal();
		void renderEditing();
		void processBloom(int passes);
		void handleCameraMovement(float xOffset, float yOffset);
		void handleCameraZoom(float yOffset);
		void updateFramebufferSize(int fbWidth, int fbHeight);
		void updateWindowSize(int width, int height);
		void selectBody();

		void addBody(std::unique_ptr<Body> body);
		void removeBody(unsigned int bodyID);
		unsigned int pickObject(const Camera::Ray& ray);
		GLFWwindow* getWindow();

		enum state {
			NORMAL, EDITING
		};

		enum editorState {
			EDITING_POSITION, EDITING_VELOCITY, CREATED
		};

		state currentState;
		editorState currentEditorState;
};
