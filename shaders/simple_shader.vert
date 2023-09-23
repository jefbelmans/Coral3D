#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec2 inTexcoord;

layout (location = 0) out struct VS_OUT
{
	vec3 normal;
	vec4 tangent;
	vec3 bitangent;
	vec2 texcoord;
	vec3 viewDir;
	vec3 lightDir;
	mat3 TBN;
} vs_out;

layout (set = 0, binding = 0) uniform GlobalUBO
{
	// MATRICES
	mat4 view;
	mat4 viewInverse;
    mat4 viewProjection;

	// GLOBAL LIGHT
	vec4 globalLightDirection;
	vec4 ambientLightColor;
	vec4 cameraPos;
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
	vs_out.tangent = inTangent;
	vs_out.bitangent = inBitangent;
	vs_out.texcoord = inTexcoord;

	vs_out.lightDir = ubo.globalLightDirection.xyz;
	vs_out.viewDir = (worldPos.xyz - ubo.cameraPos.xyz);

	vec3 T = normalize(mat3(primitive.model) * inTangent.xyz);
	vec3 B = normalize(mat3(primitive.model) * inBitangent);
	vec3 N = normalize(mat3(primitive.model) * inNormal);

	vs_out.TBN = mat3(T, B, N);
}