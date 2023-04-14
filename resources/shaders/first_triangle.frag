#version 460

layout (location = 0) in vec3 vOut_color;
layout (location = 1) in vec2 vOut_texCoord;

layout (binding = 1) uniform sampler2D checkerSampler;

layout (location = 0) out vec4 pOut_color;

void main()
{
    //pOut_color = vec4(vOut_color, 1);
    //pOut_color = vec4(vOut_texCoord, 0, 1);
    pOut_color = vec4(vOut_color * texture(checkerSampler, vOut_texCoord).rgb, 1);
}
