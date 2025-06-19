#version 330 core

layout(std140) uniform CommonUniforms
{
    mat4 projection;
    mat4 view;
} commonUniforms;

layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = commonUniforms.projection * commonUniforms.view * vec4(aPos, 1.0);
}