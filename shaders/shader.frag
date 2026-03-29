#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

uniform sampler2D tex;
uniform vec3 viewPos;
uniform vec3 colour;
uniform bool emissive;

struct Light
{
    // main attributes
    vec3 position;
    vec3 colour;

    // attenuation
    float constant;
    float linear;
    float quadratic;
};

#define MAX_LIGHTS 10
uniform int emissiveCount;
uniform Light emissiveBodies[MAX_LIGHTS];

vec3 calculateLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    const float ambientStrength = 0.1;
    const float diffuseStrength = 0.8;
    const float specularStrength = 0.0;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 

    vec3 ambient = light.colour * ambientStrength * attenuation;

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.colour * diffuseStrength * diff * attenuation;

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
    vec3 specular = light.colour * specularStrength * spec * attenuation;

    return colour * (ambient + diffuse + specular);
}

void main()
{
    if (emissive) {
        FragColor = vec4(colour, 1.0);
        BloomColor = vec4(colour * 3.0, 1.0);
        return;
    }

    else if (emissiveCount == 0) {
        FragColor = vec4(colour * 0.25, 1.0);
        return;
    }

    else {
        vec3 norm = normalize(normal);
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 result = vec3(0.0);

        for (int i = 0; i < emissiveCount; i++)
        {
            result += calculateLight(emissiveBodies[i], norm, fragPos, viewDir);
        }
        
        FragColor = vec4(result, 1.0);
        BloomColor = vec4(0.0);
        return;
    }
}
