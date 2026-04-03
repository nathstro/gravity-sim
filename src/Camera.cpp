#include "Camera.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "imgui.h"

Camera::Camera()
: 	cameraPos(glm::vec3(0.0f, 0.0f, 0.0f)),
	cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
	cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	yaw(-90.0f),
	pitch(0.0f),
	fov(55.0f),
	CAM_SPEED(5.5f),
	SENSITIVITY(0.1f),
	WORLD_UP(glm::vec3(0.0f, 1.0f, 0.0f))
{
	updateCameraVectors();
}

void Camera::showButtons()
{
	ImGui::SliderFloat("Cam Speed", &CAM_SPEED, 3.0f, 30.0f);
	ImGui::SliderFloat("Sensitivity", &SENSITIVITY, 0.05, 0.5);
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

void Camera::handleScroll(float yOffset)
{
	fov -= yOffset;
	if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}

void Camera::setPosition(float x, float y, float z)
{
	cameraPos = glm::vec3(x, y, z);
}

void Camera::setPosition(glm::vec3 position)
{
	cameraPos = position;
}

void Camera::setPitch(float pitch)
{
	this->pitch = pitch;
}

void Camera::setYaw(float yaw)
{
	this->yaw = yaw;
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(cameraPos, cameraPos + cameraFront, WORLD_UP);
}

glm::mat4 Camera::getProjectionMatrix(int fbWidth, int fbHeight) const
{
	return glm::perspective(glm::radians(fov), (float)fbWidth / fbHeight, 0.1f, 500.0f);
}

glm::vec3 Camera::getPosition() const
{
	return cameraPos;
}

float Camera::getYaw() const
{
	return yaw;
}

float Camera::getPitch() const
{
	return pitch;
}

float Camera::getFOV() const
{
	return fov;
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

Camera::Ray Camera::getRay(float mouseX, float mouseY, int width, int height) const
{
	Ray r;
	r.origin = getPosition();

	// convert to ndc
	float xPos = (2.0f * mouseX) / width - 1.0f;
	float yPos = 1.0f - (2.0f * mouseY) / height;

	glm::vec4 rayClip(xPos, yPos, -1.0f, 1.0f);

	glm::vec4 rayEye = glm::inverse(getProjectionMatrix(width, height)) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

	glm::vec3 rayWorld(glm::inverse(getViewMatrix()) * rayEye);
	
	r.direction = glm::normalize(rayWorld);
	return r;
}




