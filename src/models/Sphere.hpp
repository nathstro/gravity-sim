#pragma once
#include <vector>

class Sphere
{
	public:
		Sphere(float radius, int sectors, int stacks);
		const std::vector<float>& getVertices();
		const std::vector<unsigned int>& getIndices();
	private:
		float radius;
		int sectors;
		int stacks;
		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		void buildVertices();
};