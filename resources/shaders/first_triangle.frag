#version 460

layout (location = 0) in vec3 vOut_color;

layout (location = 0) out vec4 pOut_color;

void main()
{
    pOut_color = vec4(vOut_color, 1);
}
