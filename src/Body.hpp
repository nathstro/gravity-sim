#pragma once
#include <glm/glm.hpp>

struct Body
{
	glm::vec3 position;
	float mass;
	float radius;

	Body(glm::vec3 position, float mass, float radius);
};