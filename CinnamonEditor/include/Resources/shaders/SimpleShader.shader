#vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TextureCoordinates;

layout(location = 0) out vec2 v_TextureCoordinates;

void main()
{
	gl_Position = vec4(a_Position, 1.0);
	v_TextureCoordinates = a_TextureCoordinates;
}

#fragment
#version 450 core

layout(binding = 0) uniform sampler2D u_Texture;

layout(location = 0) in vec2 v_TextureCoordinates;
layout(location = 0) out vec4 Color;

void main()
{	
	Color = texture(u_Texture, v_TextureCoordinates);
}