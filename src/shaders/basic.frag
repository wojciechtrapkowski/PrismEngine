#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform CommonUniforms
{
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
}
commonUniforms;

layout(set = 0, binding = 1) uniform sampler2D textures[];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextureUV;
layout(location = 3) in flat int inTextureId;

layout(location = 0) out vec4 FragColor;

void main()
{
    const vec3  lightColor      = vec3(1.0, 1.0, 1.0);
    const float ambientStrength = 0.3;

    vec3 normal = normalize(inNormal);

    vec3 lightDir = normalize(vec3(commonUniforms.cameraPosition) - inPosition);
    vec3 diffuse  = max(dot(normal, lightDir), 0.0) * lightColor;

    vec3 ambient = ambientStrength * lightColor;

    vec4 objectColor = vec4(1.0, 0.0, 0.0, 1.0);

    if (inTextureId != -1) {
        objectColor = texture(textures[nonuniformEXT(inTextureId)], inTextureUV);
    }
    vec3 result = (ambient + diffuse) * vec3(objectColor);

    FragColor = vec4(result, 1.0);
}