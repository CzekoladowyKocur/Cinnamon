#vertex
#version 450

layout(location = 0) out vec2 v_TextureCoordinates;

void main()
{
	v_TextureCoordinates = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(v_TextureCoordinates * 2.0f - 1.0f, 0.0f, 1.0f);
}

#fragment
#version 450

layout(set = 0, binding = 0) uniform sampler2D u_Texture;

layout(location = 0) in vec2 v_TextureCoordinates;
layout(location = 0) out vec4 Color;

void main()
{
	vec4 albedo = texture(u_Texture, v_TextureCoordinates);
	Color = albedo;
}