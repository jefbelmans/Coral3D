#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUV;

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

layout (binding = 1) uniform sampler2D texSampler;

layout (push_constant) uniform Push
{
	mat4 worldMatrix;
	mat4 normalMatrix;
} push;

// HELPERS
vec3 calculate_diffuse(vec3 col, vec3 norm)
{
	float diffuseStrength = dot(norm, -ubo.globalLightDirection.xyz);

	// HALF-LAMBERT
	diffuseStrength = diffuseStrength * 0.5f + 0.5f;
	diffuseStrength = clamp(diffuseStrength, 0.f, 1.f);

	return col * diffuseStrength;
}

void main()
{
	// POINT LIGHT
	// vec3 directionToLight = ubo.lightPosition.xyz - fragPosWorld;
	// float attenuation = 1.0f / dot(directionToLight, directionToLight);
	// vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;

	vec3 ambient = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 color = texture(texSampler, fragUV).xyz;
	vec3 diffuse = calculate_diffuse(color, fragNormalWorld);

	outColor = vec4(diffuse, 1.0f);
}