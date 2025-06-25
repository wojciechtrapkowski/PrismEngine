#version 330 core

layout(std140) uniform CommonUniforms {
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
} commonUniforms;


in vec3 outPosition;
in vec3 outNormal;

out vec4 FragColor;

void main()
{
    const vec3 lightColor = vec3(1.0, 1.0, 1.0);
    const float ambientStrength = 0.2;

    vec3 normal = normalize(outNormal);
    
    vec3 lightDir = normalize(vec3(commonUniforms.cameraPosition) - outPosition);
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * lightColor;

    vec3 ambient = ambientStrength * lightColor;

    vec4 objectColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    vec3 result = (ambient + diffuse) * vec3(objectColor);

    FragColor = vec4(result, 1.0);
}