#import "Sphere.hpp"

Sphere::Sphere(float radius, int sectors, int stacks) : radius(radius), sectors(sectors), stacks(stacks)
{
	buildVertices();
}

void Sphere::buildVertices()
{
    const float PI = 3.14159265359f;
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= stacks; ++i)
    {
        float stackAngle = PI/2 - i * PI/stacks; // from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j)
        {
            float sectorAngle = j * 2 * PI / sectors;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normalized for lighting
            float lengthInv = 1.0f / radius;
            vertices.push_back(x * lengthInv);
            vertices.push_back(y * lengthInv);
            vertices.push_back(z * lengthInv);

            // Optional texcoords
            vertices.push_back((float)j / sectors);
            vertices.push_back((float)i / stacks);
        }
    }

    // generate indices
    for(int i = 0; i < stacks; ++i)
    {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for(int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            if(i != 0)
                indices.push_back(k1), indices.push_back(k2), indices.push_back(k1 + 1);
            if(i != (stacks - 1))
                indices.push_back(k1 + 1), indices.push_back(k2), indices.push_back(k2 + 1);
        }
    }
}

const std::vector<float>& Sphere::getVertices()
{
	return vertices;
}

const std::vector<unsigned int>& Sphere::getIndices()
{
	return indices;
}
