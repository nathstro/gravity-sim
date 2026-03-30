#pragma once
#include <glm/glm.hpp>
#include <string>

struct Body
{
	glm::vec3 previousPosition;
	glm::vec3 position;
	glm::vec3 acceleration;
	glm::vec3 colour;

	float mass;
	float radius;
	bool emissive;
	std::string name;

	Body(std::string name, glm::vec3 position, glm::vec3 velocity, glm::vec3 colour, float mass, float radius, bool emissive);
	float getVelocity();
	float getAcceleration();
};