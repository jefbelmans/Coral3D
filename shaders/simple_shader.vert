#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec2 inTexcoord;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec4 outTangent;
layout (location = 2) out vec2 outTexcoord;
layout (location = 3) out vec3 outViewDir;
layout (location = 4) out vec3 outLightDir;
layout (location = 5) out vec3 outBitangent;

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

	mat3 modelM3 = mat3(primitive.model);
	outNormal = modelM3 * inNormal;
	outTangent = vec4(normalize(modelM3 * inTangent.xyz), inTangent.w);
	outBitangent = modelM3 * inBitangent;
	outTexcoord = inTexcoord;

	outLightDir = normalize(ubo.globalLightDirection.xyz);
	outViewDir = normalize(ubo.cameraPos.xyz - worldPos.xyz);
}