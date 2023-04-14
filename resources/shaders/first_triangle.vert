#version 460

// Inputs
layout (location = 0) in vec3 vIn_position;
layout (location = 1) in vec3 vIn_color;
layout (location = 2) in vec2 vIn_texCoord;

// Outputs
layout (location = 0) out vec3 vOut_color;
layout (location = 1) out vec2 vOut_texCoord;

// Uniforms
layout(set = 0, binding = 0) uniform FrameData
{
    mat4 view;
    mat4 proj;
} ub_FrameData;

void main()
{
    gl_Position = ub_FrameData.proj * ub_FrameData.view * vec4(vIn_position, 1);
    //gl_Position = vec4(vIn_position, 1);
    vOut_color = vIn_color;
    vOut_texCoord = vIn_texCoord;
}
