#version 330 core
out vec4 FragColour;

in float offset;
uniform float time;

void main() {
	float brightness = 0.3f + 0.7f * ((sin(time + offset) + 1.0f) / 2.0f);
	FragColour = vec4(brightness);
}