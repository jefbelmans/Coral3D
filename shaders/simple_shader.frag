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
	vec3 normal;
	vec4 tangent;
	vec3 bitangent;
	vec2 texcoord;
	vec3 viewDir;
	vec3 lightDir;
} fs_in;

// OUT COLOR
layout (location = 0) out vec4 outFragColor;

vec3 calculate_diffuse(vec3 color, vec3 normal)
{
	float diffuse_strength = dot(normal, -normalize(fs_in.lightDir));

	// HALF-LAMBERT
	diffuse_strength = diffuse_strength * 0.5f + 0.5f;
	diffuse_strength = clamp(diffuse_strength, 0.f, 1.f);

	return color * diffuse_strength;
}

vec3 calculate_specular(vec3 V, vec3 L, vec3 normal)
{
	// HALF VECTOR
	vec3 halfVector = reflect(-L, normal);
	float specularStrength = clamp(dot(halfVector, V), 0.f, 1.0f);

	// SHININESS
	float exp = 15.f;
	float specularity = pow(specularStrength, exp);

	vec3 specularColor = vec3(1.f, 1.f, 1.f);

	return specularColor * specularity;
}

void main()
{
	vec4 color = texture(samplerColorMap, fs_in.texcoord);

	if (ALPHA_MASK) {
		if (color.a < ALPHA_MASK_CUTOFF) {
			discard;
		}
	}

	vec3 localNormal = 2.f * texture(samplerNormalMap, fs_in.texcoord).rgb - 1.f;
	vec3 normal = normalize(localNormal);

	vec3 diffuse = calculate_diffuse(color.rgb, normal);

	outFragColor = vec4(diffuse, color.a);
}