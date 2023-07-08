#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

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
	vec3 directionToLight = ubo.lightPosition.xyz - fragPosWorld;
	float attenuation = 1.0f / dot(directionToLight, directionToLight);

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambient = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;

	// HALF-LAMBERT DIFFUSE
	float lightIntensity = max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0.0f);
	lightIntensity = lightIntensity * 0.5f + 0.5f;
	vec3 diffuse = lightColor * lightIntensity;

	outColor = vec4((diffuse + ambient) * fragColor, 1.0f);
}