#include "Body.hpp"
#include <random>
#include <string>

std::random_device rd;
std::mt19937 rng(rd());
std::normal_distribution<float> dist(0.75, 0.2);

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

void Body::recalcVelocity(glm::vec3 velocity)
{
	previousPosition = position - (velocity * 0.016f);
}

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