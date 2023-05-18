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

layout(location = 0) in vec2 v_TextureCoordinates;
layout(location = 0) out vec4 Color;

layout(set = 0, binding = 0) uniform sampler2D u_PositionSampler;
layout(set = 0, binding = 1) uniform sampler2D u_AlbedoSampler;

struct Light
{
	vec4 Color;
	vec3 Position;
	float Intensity;
};

layout(set = 0, binding = 2) readonly uniform LightBufferStruct
{
	int LightCount;
	vec3 AmbientLight;
	Light Lights[100];
} LightBuffer;

const vec3 fragNormal = vec3(0.0, 0.0, 1.0);

void main()
{
	vec3 fragmentPosition	= texture(u_PositionSampler, v_TextureCoordinates).rgb;
	vec4 albedo				= texture(u_AlbedoSampler, v_TextureCoordinates);
	vec3 finalColor			= vec3(0.0);
	
	for (int i = 0; i < LightBuffer.LightCount; ++i)
	{
		vec3 lightDirection = normalize(LightBuffer.Lights[i].Position - fragmentPosition);

		float diffuseFactor = max(dot(fragNormal, lightDirection), 0.0);

		vec3 diffuseColor = diffuseFactor * albedo.xyz * LightBuffer.Lights[i].Color.xyz * LightBuffer.Lights[i].Intensity;

		finalColor += diffuseColor * albedo.w;
	}

	const vec4 ambient = vec4(0.0, 0.0, 0.0, 1.0);
	Color = vec4(LightBuffer.AmbientLight + finalColor, 1.0);
}