#version 330 core
out vec4 FragColour;

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

uniform sampler2D tex;

void main()
{
    /*vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;

    vec3 result = ambient + diffuse + specular;*/
    // FragColour = mix(texture(tex, texCoord), vec4(result, 1.0f), 0.8f);
    FragColour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
