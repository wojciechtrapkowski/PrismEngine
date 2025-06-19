#version 330 core

layout(std140) uniform CommonUniforms
{
    mat4 projection;
    mat4 view;
} commonUniforms;

uniform mat4 model;

layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = commonUniforms.projection * commonUniforms.view *  model * vec4(aPos, 1.0);
}