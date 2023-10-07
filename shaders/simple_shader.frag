#version 450

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
	int numLights;
} ubo;

// SAMPLERS
layout (set = 0, binding = 1) uniform samplerCube samplerCubeMap;
layout (set = 1, binding = 0) uniform sampler2D samplerColorMap;
layout (set = 1, binding = 1) uniform sampler2D samplerNormalMap;

// CONSTANTS
layout (constant_id = 0) const bool ALPHA_MASK = false;
layout (constant_id = 1) const float ALPHA_MASK_CUTOFF = 0.0f;

// FRAGMENT INPUT
layout (location = 0) in struct FS_IN
{
	vec3 fragPos;
	vec2 texcoord;
	mat3 TBN;
} fs_in;

// OUT COLOR
layout (location = 0) out vec4 outFragColor;

vec3 calculate_diffuse(vec3 N, vec3 L, vec3 C)
{
	float diffuse_strength = max(dot(N, L), 0);

	// HALF-LAMBERT
	diffuse_strength = pow(diffuse_strength * 0.5f + 0.5f, 2);
	diffuse_strength = max(diffuse_strength, 0.f);

	return C * diffuse_strength;
}

vec3 calculate_specular(vec3 N, vec3 V, vec3 L, vec3 C)
{
	V = normalize(V);
	L = normalize(L);

	// HALF VECTOR
	vec3 halfVector = normalize(L + V);

	// Specularity
	float shininess = 16.f;
	float specularity = pow(max(dot(N, halfVector), 0.f), shininess * shininess);

	return C * specularity;
}

vec3 calculate_fresnel(vec3 N, vec3 V, vec3 C)
{
	float fresnelPower = 1.f;
	float fresnelMultiplier = 1.f;
	float fresnelHardness = 0.f;

	float fresnel = pow(1.f - max(abs(dot(N, V)), 0), fresnelPower) * fresnelMultiplier;
	vec3 fresnelMask = pow(1.f - max(vec3(0.0f, -1.0f, 0.0f) * N, 0.f), vec3(fresnelHardness));

	return fresnel * fresnelMask * C;
}

void main()
{
	// BASE COLOR
	vec4 color = texture(samplerColorMap, fs_in.texcoord);

	if (ALPHA_MASK)
		if (color.a < ALPHA_MASK_CUTOFF) discard;

	// SAMPLE NORMAL
	vec3 N = texture(samplerNormalMap, fs_in.texcoord).rgb;
	N = normalize(fs_in.TBN * (N * 2.f - 1.f));

	// VECTORS
	vec3 V = normalize(ubo.viewInverse[3].xyz - fs_in.fragPos);
	vec3 R = reflect(V, N);

	// GLOBAL LIGHT
	vec3 ambient = ubo.ambientLighting.xyz * ubo.ambientLighting.w;
	vec3 diffuse = calculate_diffuse(N, ubo.globalLightDirection.xyz, color.rgb * ubo.globalLightDirection.w);
	vec3 specular = calculate_specular(N, V,  ubo.globalLightDirection.xyz, vec3(1,1,1) *  ubo.globalLightDirection.w);

	// POINT LIGHTS
	for(int i = 0; i < ubo.numLights; i++)
	{
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fs_in.fragPos;
		float attenuation = 1.f / dot(directionToLight, directionToLight);
		vec3 lightColor = light.color.xyz * color.rgb * light.color.w * attenuation;

		diffuse += calculate_diffuse(N, directionToLight, lightColor);
		specular += calculate_specular(N, V, directionToLight, lightColor);
	}

	vec3 environment = texture(samplerCubeMap, R).rgb;
	environment = calculate_fresnel(N, V, environment);
	outFragColor = vec4(ambient + diffuse + specular + environment, color.a);
}