#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 color;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragPosWorld;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec2 fragUV;

layout (set = 0, binding = 0) uniform GlobalUBO
{
    mat4 viewProjection;

	// GLOBAL LIGHT
	vec4 globalLightDirection;
	vec4 ambientLightColor;
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

	fragColor = color;
	fragPosWorld = worldPos.xyz;
	fragNormal = normal;
	fragUV = uv;
}