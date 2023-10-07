#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inTangent;
layout (location = 3) in vec2 inTexcoord;

layout (location = 0) out struct VS_OUT
{
	vec3 fragPos;
	vec2 texcoord;
	mat3 TBN;
} vs_out;

struct PointLight
{
	vec4 position; // w is radius
	vec4 color; // w is intensity
};

layout (set = 0, binding = 0) uniform GlobalUBO
{
	// MATRICES
	mat4 view;
	mat4 viewInverse;
	mat4 viewProjection;

	// LIGHTING
	vec4 globalLightDirection;
	vec4 ambientLighting;

	// POINT LIGHTS
	PointLight pointLights[8];
	int numLights;
} ubo;

layout (push_constant) uniform Push
{
	mat4 model;
} primitive;

void main() 
{
	vec4 worldPos = primitive.model * vec4(inPosition, 1.0f);
	gl_Position = ubo.viewProjection * worldPos;

	vs_out.fragPos  = worldPos.xyz;
	vs_out.texcoord = inTexcoord;

	// TBN
	vec3 T = normalize(mat3(primitive.model) * inTangent.xyz);
	vec3 N = normalize(mat3(primitive.model) * inNormal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T) * inTangent.w;

	vs_out.TBN = mat3(T, B, N);
}