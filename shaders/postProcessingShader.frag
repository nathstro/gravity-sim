#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;

void main()
{             
    const float exposure = 1.0;
    const float gamma = 2.2;

    vec3 hdrColour = texture(screenTexture, TexCoords).rgb;
    vec3 bloomColour = texture(bloomTexture, TexCoords).rgb;
    hdrColour += bloomColour;

    vec3 result = vec3(1.0) - exp(-hdrColour * exposure);
        
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}
