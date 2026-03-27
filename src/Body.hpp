#pragma once
#include <glm/glm.hpp>

struct Body
{
	glm::vec3 previousPosition;
	glm::vec3 position;
	glm::vec3 acceleration;
	glm::vec3 colour;
	std::vector<glm::vec3> trail;

	float mass;
	float radius;

	Body(glm::vec3 position, glm::vec3 velocity, glm::vec3 colour, float mass, float radius);
};