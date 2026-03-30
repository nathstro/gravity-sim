#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum class CameraMove
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
	public:
		struct Ray {
			glm::vec3 origin;
			glm::vec3 direction;
		};

		Camera(glm::vec3 initialCameraPos);
		void handleKeyboard(CameraMove direction, float deltaTime);
		void handleMouseMovement(float xOffset, float yOffset);
		glm::mat4 getProjectionMatrix(int fbWidth, int fbHeight) const;
		glm::mat4 getViewMatrix() const;
		glm::vec3 getPosition() const;
		Ray getRay(float mouseX, float mouseY, int width, int height) const;
		
	private:
		const float CAM_SPEED;
		const float SENSITIVITY;
		const float FOV;
		const glm::vec3 WORLD_UP;

		glm::vec3 cameraPos;
		glm::vec3 cameraFront;
		glm::vec3 cameraUp;
		glm::vec3 cameraRight;
		float yaw;
		float pitch;

		void updateCameraVectors();
};