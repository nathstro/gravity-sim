#include "Body.hpp"
#include <string>

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

float Body::getVelocity()
{
	return glm::length((position - previousPosition) / 0.016f);
}

float Body::getAcceleration()
{
	return glm::length(acceleration);
}

// where to put velocity??
// velocity = s1 - s0 / t
// s1 - vt = s0