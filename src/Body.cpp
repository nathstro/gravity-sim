#include "Body.hpp"
#include <random>
#include <string>

std::random_device rd;
std::mt19937 rng(rd());
std::normal_distribution<float> dist(0.75, 0.2);

Body::Body()
:	name("New Body"),
	position(glm::vec3(0.0f)),
	previousPosition(glm::vec3(0.0f)),
	acceleration(glm::vec3(0.0f)),
	colour(glm::vec3(1.0f)),
	mass(1.0f),
	radius(1.0f),
	emissive(false)
{}

Body::Body(std::string name, glm::vec3 position, glm::vec3 velocity, glm::vec3 colour, float mass, float radius, bool emissive)
: 	name(name),
	position(position),
	previousPosition(position - (velocity * 0.016f)),
	acceleration(glm::vec3(0.0f)),
	colour(colour),
	mass(mass),
	radius(radius),
	emissive(emissive) 
{}

void Body::setVelocity(glm::vec3 velocity, float timeStep)
{
	previousPosition = position - (velocity * timeStep);
}

float Body::getDisplacement(glm::vec3 origin)
{
	return glm::length(position - origin);
}

glm::vec3 Body::getVelocity()
{
	return (position - previousPosition) / 0.016f;
}

// where to put velocity??
// velocity = s1 - s0 / t
// s1 - vt = s0