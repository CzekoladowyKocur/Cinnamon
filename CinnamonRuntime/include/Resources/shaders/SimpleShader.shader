#vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TextureCoordinates;
layout(location = 3) in float a_TilingFactor;

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TextureCoordinates;
layout(location = 2) out float v_TilingFactor;

layout (set = 0, binding = 0) uniform UniformBuffer 
{
    mat4 Camera;
} UBO;

void main()
{
	gl_Position = UBO.Camera * vec4(a_Position, 1.0);
	v_TextureCoordinates = a_TextureCoordinates;
	v_Color = a_Color;
	v_TilingFactor = a_TilingFactor;
}

#fragment
#version 450 core

layout(binding = 1) uniform sampler2D u_Texture;

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TextureCoordinates;
layout(location = 2) in float v_TilingFactor;

layout(location = 0) out vec4 Color;

void main()
{	
	Color = v_Color * texture(u_Texture, v_TextureCoordinates * v_TilingFactor);
}