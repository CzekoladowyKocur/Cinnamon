#vertex 
#version 450 core

// First descriptor set: Uniform buffer with Mat4 matrix
layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4 Camera;
} UBO;

void main() 
{
    gl_Position = UBO.Camera * vec4(0.0, 0.0, 0.0, 1.0);
}

#fragment
#version 450 core

// Second descriptor set: Texture
layout(set = 1, binding = 0) uniform sampler2D u_TextureSampler;
layout(set = 1, binding = 1) uniform sampler2D u_TextureSampler2;

layout(location = 0) out vec4 Color;

void main()
{
    Color = vec4(1.0, 1.0, 1.0, 1.0);
}
