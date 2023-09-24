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
	mat3 TBN;
} fs_in;

// OUT COLOR
layout (location = 0) out vec4 outFragColor;

vec3 calculate_diffuse(vec3 color, vec3 N, vec3 L)
{
	float diffuse_strength = clamp(dot(N, -L), 0, 1);

	// HALF-LAMBERT
	diffuse_strength = diffuse_strength * 0.5f + 0.5f;
	diffuse_strength = clamp(diffuse_strength, 0.f, 1.f);

	return color * diffuse_strength;
}

vec3 calculate_specular(vec3 V, vec3 L, vec3 normal)
{
	// HALF VECTOR
	vec3 halfVector = reflect(-normalize(L), normal);
	float specularStrength = clamp(dot(halfVector, normalize(V)), 0.f, 1.0f);

	// SHININESS
	float exp = 30.f;
	float specularity = pow(specularStrength, exp);

	vec3 specularColor = vec3(1.f, 1.f, 1.f);

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

	outFragColor = vec4(diffuse, color.a);
}