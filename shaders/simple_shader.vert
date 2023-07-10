#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal;

layout (location = 0) out vec3 fragPosWorld;
layout (location = 1) out vec2 fragTexcoord;
layout (location = 2) out vec3 fragNormalWorld;

layout (set = 0, binding = 0) uniform GlobalUBO
{
    mat4 viewProjection;

	// GLOBAL LIGHT
	vec4 globalLightDirection;
	vec4 ambientLightColor;

	// POINT LIGHT
	vec4 lightPosition;
	vec4 lightColor;
} ubo;

layout (push_constant) uniform Push
{
	mat4 worldMatrix;
	mat4 normalMatrix;
} push;

void main() 
{
	vec4 worldPos = push.worldMatrix * vec4(position, 1.0f);
	gl_Position = ubo.viewProjection * worldPos;

	fragPosWorld = worldPos.xyz;
	fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
	fragTexcoord = texcoord;
}