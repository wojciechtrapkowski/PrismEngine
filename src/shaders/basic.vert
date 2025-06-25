#version 330 core

layout(std140) uniform CommonUniforms {
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
}

commonUniforms;

uniform mat4 model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

out vec3 outPosition;
out vec3 outNormal;

void main() {
    gl_Position = commonUniforms.projection * commonUniforms.view * model *
                  vec4(inPosition, 1.0);
    outNormal = inNormal;
    outPosition = vec3(model * vec4(inPosition, 1.0));
}