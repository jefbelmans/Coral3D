#version 450

// SAMPLERS
layout (set = 1, binding = 0) uniform sampler2D samplerColorMap;
layout (set = 1, binding = 1) uniform sampler2D samplerNormalMap;

// CONSTANTS
layout (constant_id = 0) const bool ALPHA_MASK = false;
layout (constant_id = 1) const float ALPHA_MASK_CUTOFF = 0.0f;

// FRAGMENT INPUT
layout (location = 0) in struct FS_IN
{
	vec3 fragPos;
	vec3 normal;
	vec2 texcoord;
	vec3 viewDir;
	vec3 lightDir;
	vec4 ambient;
} fs_in;

struct PointLight
{
	vec4 position; // w is radius
	vec4 color; // w is intensity
};

layout (set = 0, binding = 0) uniform GlobalUBO
{
	// MATRICES
	mat4 view;
	mat4 viewInverse;
	mat4 viewProjection;

	// LIGHTING
	vec4 globalLightDirection;
	vec4 ambientLighting;

	// POINT LIGHTS
	PointLight pointLights[8];
	float numLights;
} ubo;

// OUT COLOR
layout (location = 0) out vec4 outFragColor;

vec3 calculate_diffuse(vec3 color, vec3 N, vec3 L)
{
	float diffuse_strength = max(dot(N, L), 0);

	// HALF-LAMBERT
	diffuse_strength = pow(diffuse_strength * 0.5f + 0.5f, 2);
	diffuse_strength = max(diffuse_strength, 0.f);

	return color * diffuse_strength;
}

vec3 calculate_specular(vec3 V, vec3 L, vec3 N, vec3 C)
{
	V = normalize(V);
	L = normalize(L);

	// HALF VECTOR
	vec3 halfVector = normalize(L + V);

	// Specularity
	float shininess = 32.f;
	float specularity = pow(max(dot(N, halfVector), 0.f), shininess * shininess);

	return C * specularity;
}

void main()
{
	// BASE COLOR
	vec4 color = texture(samplerColorMap, fs_in.texcoord);

	if (ALPHA_MASK)
		if (color.a < ALPHA_MASK_CUTOFF) discard;

	// SAMPLE NORMAL
	vec3 normal = texture(samplerNormalMap, fs_in.texcoord).rgb;
	normal = normalize(normal * 2.f - 1.f);

	// POINT LIGHTS
	vec3 diffuse = ubo.ambientLighting.xyz * ubo.ambientLighting.w;
	vec3 specular = vec3(0,0,0);
	for(int i = 0; i < ubo.numLights; i++)
	{
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xzy - fs_in.fragPos.xyz;
		float attenuation = 1.f / dot(directionToLight, directionToLight);
		vec3 lightColor = light.color.xyz * light.color.w * attenuation;

		diffuse += calculate_diffuse(lightColor, normal, directionToLight);
		specular += calculate_specular(fs_in.viewDir, directionToLight, normal, light.color.xyz * light.color.w);
	}

	outFragColor = vec4(color.xyz * diffuse + specular, color.a);
}