#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aStarSize;
layout (location = 2) in float aOffset;

out float offset;

uniform mat4 projection;
uniform mat4 view;

void main() {
	gl_PointSize = aStarSize;
	gl_Position = projection * view * vec4(aPos, 1.0f);
	offset = aOffset;
}