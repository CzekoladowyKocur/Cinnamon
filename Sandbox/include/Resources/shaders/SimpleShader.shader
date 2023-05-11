#vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

void main()
{
	gl_Position = vec4(a_Position, 1.0);
}

#fragment
#version 450 core

layout(location = 0) out vec4 Color;
void main()
{	
	Color = vec4(0.2, 0.0, 1.0, 1.0);
}