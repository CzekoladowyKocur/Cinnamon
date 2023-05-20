#vertex
#version 450 core

layout(std140, binding = 0) uniform UniformBufferObject
{
	mat4 model;
} UBO;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(location = 0) out vec4 v_Color;

void main()
{
	gl_Position = UBO.model * vec4(a_Position, 1.0);
	gl_Position.y = -gl_Position.y;
	v_Color = a_Color;
}

#fragment
#version 450 core

layout(location = 0) in vec4 v_Color;
layout(location = 0) out vec4 Color;

void main()
{
	Color = v_Color;
}