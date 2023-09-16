#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inTangent;
layout (location = 3) in vec2 inTexcoord;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec4 outTangent;
layout (location = 2) out vec2 outTexcoord;
layout (location = 3) out vec3 outViewDir;
layout (location = 4) out vec3 outLightDir;
layout (location = 5) out mat3 outTBN;

layout (set = 0, binding = 0) uniform GlobalUBO
{
	// MATRICES
	mat4 view;
	mat4 viewInverse;
    mat4 viewProjection;

	// GLOBAL LIGHT
	vec4 globalLightDirection;
	vec4 ambientLightColor;
} ubo;

layout (push_constant) uniform Push
{
	mat4 model;
} primitive;

void main() 
{
	outNormal = inNormal;
	outTangent = inTangent;
	outTexcoord = inTexcoord;

	vec4 worldPos = primitive.model * vec4(inPosition, 1.0f);
	gl_Position = ubo.viewProjection * worldPos;

	outNormal = normalize(mat3(primitive.model) * inNormal);
	outLightDir = ubo.globalLightDirection.xyz;
	outViewDir = ubo.viewInverse[3].xyz - worldPos.xyz;

	vec3 T = normalize(vec3(primitive.model * vec4(inTangent.xyz * inTangent.w, 0.f)));
	vec3 N = normalize(vec3(primitive.model * vec4(inNormal, 0.f)));
	vec3 B = normalize(cross(N, T));
	outTBN = mat3(T, B, N);
}