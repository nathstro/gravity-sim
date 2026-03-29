#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;

void main()
{             
    const float exposure = 1.0;
    const float gamma = 2.2;

    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    vec3 glow = texture(bloomTexture, TexCoords).rgb;

    vec3 combined = hdrColor + glow * 1.5;
    vec3 mapped = vec3(1.0) - exp(-combined * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}
