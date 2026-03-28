#include "Body.hpp"

Body::Body(glm::vec3 position, glm::vec3 velocity, glm::vec3 colour, float mass, float radius, bool emissive)
: position(position), previousPosition(position - (velocity * 0.016f)), acceleration(glm::vec3(0.0f)), colour(colour), mass(mass), radius(radius), emissive(emissive)
{}

// where to put velocity??
// velocity = s1 - s0 / t
// s1 - vt = s0