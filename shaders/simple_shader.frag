#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform GlobalUBO
{
    mat4 viewProjection;

	// GLOBAL LIGHT
	vec4 globalLightDirection;
	vec4 ambientLightColor;
} ubo;

layout (set = 1, binding = 0) uniform sampler2D texSampler;

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
	vec3 ambient = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 color = texture(texSampler, fragUV).xyz;
	// vec3 color = { 0.6f, 0.6f, 0.6f };
	vec3 diffuse = calculate_diffuse(color, fragNormal);

	outColor = vec4(diffuse + ambient, 1.0f);
}