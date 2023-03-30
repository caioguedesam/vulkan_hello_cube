#version 460

layout (location = 0) in vec2 vIn_position;
layout (location = 1) in vec3 vIn_color;

layout (location = 0) out vec3 vOut_color;

void main()
{
    gl_Position = vec4(vIn_position, 0, 1);
    vOut_color = vIn_color;
}
