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

	mat3 modelM3 = transpose(inverse(mat3(primitive.model)));
	vec3 T = normalize(modelM3 * inTangent.xyz);
	vec3 N = normalize(modelM3 * inNormal);
	vec3 B = normalize(cross(N, T)) * inTangent.w;

	mat3 TBN = mat3(
	T.x, B.x, N.x,
	T.y, B.y, N.y,
	T.z, B.z, N.z
	);

	vs_out.normal = N;
	vs_out.tangent = vec4(T, inTangent.w);
	vs_out.bitangent = normalize(modelM3 * inBitangent);
	vs_out.texcoord = inTexcoord;

	vs_out.lightDir = normalize(ubo.globalLightDirection.xyz) * TBN;
	// vs_out.viewDir = -normalize(ubo.view * primitive.model * vec4(inPosition, 1.f)).xyz;
	vs_out.viewDir = normalize(worldPos.xyz - ubo.cameraPos.xyz) * TBN;
}