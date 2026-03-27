#include "Camera.hpp"
#include "glm/ext/matrix_transform.hpp"

Camera::Camera(glm::vec3 cameraPos)
: cameraPos(cameraPos), cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)), cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)), yaw(-90.0f), pitch(0.0f), CAM_SPEED(5.5f), SENSITIVITY(0.1f), WORLD_UP(glm::vec3(0.0f, 1.0f, 0.0f))
{
	updateCameraVectors();
}

void Camera::handleKeyboard(CameraMove direction, float deltaTime)
{
	float cameraSpeed = CAM_SPEED * deltaTime;

	switch (direction) {
		case CameraMove::FORWARD:
			cameraPos += cameraSpeed * cameraFront;
			break;
		case CameraMove::BACKWARD:
			cameraPos -= cameraSpeed * cameraFront;
			break;
		case CameraMove::LEFT:
			cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
			break;
		case CameraMove::RIGHT:
			cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
			break;
		case CameraMove::DOWN:
			cameraPos += cameraSpeed * cameraUp;
			break;
		case CameraMove::UP:
			cameraPos -= cameraSpeed * cameraUp;
			break;
	}
}

void Camera::handleMouseMovement(float xOffset, float yOffset)
{
	xOffset = xOffset * SENSITIVITY;
	yOffset = yOffset * SENSITIVITY;
	
	yaw += xOffset;
    pitch += yOffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(cameraPos, cameraPos + cameraFront, WORLD_UP);
}

glm::vec3 Camera::getPosition() const
{
	return cameraPos;
}

void Camera::updateCameraVectors()
{
	glm::vec3 cameraDirection;
    cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection.y = sin(glm::radians(pitch));
    cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(cameraDirection);
    cameraRight = glm::normalize(glm::cross(cameraFront, WORLD_UP));
    cameraUp = glm::normalize(glm::cross(cameraFront, cameraRight));
}


