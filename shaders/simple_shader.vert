#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec2 inTexcoord;

layout (location = 0) out struct VS_OUT
{
	vec3 fragPos;
	vec3 normal;
	vec4 tangent;
	vec3 bitangent;
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

	vec3 T = normalize(mat3(primitive.model) * inTangent.xyz);
	vec3 N = normalize(mat3(primitive.model) * inNormal);
	vec3 B = normalize(mat3(primitive.model) * inBitangent);

	mat3 TBN = transpose(mat3(T, B, N));

	vs_out.fragPos  = TBN * worldPos.xyz;
	vs_out.lightDir = TBN * ubo.globalLightDirection.xyz;
	vs_out.viewPos  = TBN * ubo.cameraPos.xyz;
}