#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inTangent;
layout (location = 3) in vec2 inTexcoord;

layout (location = 0) out struct VS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec2 texcoord;
	vec3 viewPos;
	vec3 lightDir;
} vs_out;

layout (set = 0, binding = 0) uniform GlobalUBO
{
	// MATRICES
	mat4 view;
	mat4 viewInverse;
    mat4 viewProjection;

	// GLOBAL LIGHT
	vec4 globalLightDirection;
} ubo;

layout (push_constant) uniform Push
{
	mat4 model;
} primitive;

void main() 
{
	vec4 worldPos = primitive.model * vec4(inPosition, 1.0f);
	gl_Position = ubo.viewProjection * worldPos;

	vs_out.normal = inNormal;
	vs_out.texcoord = inTexcoord;

	vec3 T = normalize(mat3(primitive.model) * inTangent.xyz);
	vec3 N = normalize(mat3(primitive.model) * inNormal);
	vec3 B = cross(N, T) * inTangent.w;

	mat3 TBN = transpose(mat3(T, B, N));

	vs_out.fragPos  = TBN * worldPos.xyz;
	vs_out.lightDir = TBN * ubo.globalLightDirection.xyz;
	vs_out.viewPos  = TBN * ubo.viewInverse[3].xyz;
}