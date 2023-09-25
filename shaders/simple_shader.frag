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
	vec4 tangent;
	vec3 bitangent;
	vec2 texcoord;
	vec3 viewPos;
	vec3 lightDir;
} fs_in;

// OUT COLOR
layout (location = 0) out vec4 outFragColor;

vec3 calculate_diffuse(vec3 color, vec3 N, vec3 L)
{
	float diffuse_strength = max(dot(N, -L), 0);

	// HALF-LAMBERT
	diffuse_strength = pow(diffuse_strength * 0.5f + 0.5f, 2);
	diffuse_strength = max(diffuse_strength, 0.f);

	return color * diffuse_strength;
}

vec3 calculate_specular(vec3 V, vec3 L, vec3 N)
{
	V = normalize(V);
	L = normalize(L);

	// HALF VECTOR
	vec3 halfVector = normalize(L + V);
	float shininess = 256.f;
	float specularity = pow(max(dot(N, halfVector), 0.f), shininess);

	vec3 specularColor = vec3(0.2f, 0.17f, 0.17f);

	return specularColor * specularity;
}

void main()
{
	vec4 color = texture(samplerColorMap, fs_in.texcoord);

	if (ALPHA_MASK)
	{
		if (color.a < ALPHA_MASK_CUTOFF) discard;
	}

	vec3 normal = texture(samplerNormalMap, fs_in.texcoord).rgb;
	normal = normalize(normal * 2.f - 1.f);

	vec3 diffuse = calculate_diffuse(color.rgb, normal, normalize(fs_in.lightDir));
	vec3 specular = calculate_specular(fs_in.viewPos - fs_in.fragPos, fs_in.lightDir, normal);

	outFragColor = vec4(diffuse + specular, color.a);
}