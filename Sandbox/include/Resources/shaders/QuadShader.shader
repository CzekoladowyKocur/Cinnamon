#vertex
#version 450 core

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 model;
} ubo;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TextureCoordinates;
layout(location = 3) in float a_TextureIndex;
layout(location = 4) in float a_TilingFactor;

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TextureCoordinates;
layout(location = 2) out float v_TextureIndex;
layout(location = 3) out float v_TilingFactor;

void main()
{
	gl_Position = ubo.model * vec4(a_Position, 1.0);
	v_Color = a_Color;
	v_TextureCoordinates = a_TextureCoordinates;
	v_TextureIndex = a_TextureIndex;
	v_TilingFactor = a_TilingFactor;
}

#fragment
#version 450 core
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TextureCoordinates;
layout(location = 2) in float v_TextureIndex;
layout(location = 3) in float v_TilingFactor;

layout(location = 0) out vec4 Color;

layout(binding = 1) uniform sampler2D u_Textures[32];

void main()
{	
	Color = texture(u_Textures[int(nonuniformEXT(v_TextureIndex))], v_TextureCoordinates * v_TilingFactor) * v_Color;
}