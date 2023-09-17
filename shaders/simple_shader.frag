#version 450

layout (set = 1, binding = 0) uniform sampler2D samplerColorMap;
layout (set = 1, binding = 1) uniform sampler2D samplerNormalMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec4 inTangent;
layout (location = 2) in vec2 inTexcoord;
layout (location = 3) in vec3 inViewDir;
layout (location = 4) in vec3 inLightDir;
layout (location = 5) in vec3 inBitangent;

layout (location = 0) out vec4 outFragColor;

layout (constant_id = 0) const bool ALPHA_MASK = false;
layout (constant_id = 1) const float ALPHA_MASK_CUTOFF = 0.0f;

vec3 calculate_diffuse(vec3 color, vec3 normal)
{
	float diffuse_strength = dot(normal, -inLightDir);

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
	float exp = 10.f;
	float specularity = pow(specularStrength, exp);

	vec3 specularColor = vec3(0.3f, 0.3f, 0.3f);

	return specularColor * specularity;
}

mat3 TBN;

void main()
{
	vec4 color = texture(samplerColorMap, inTexcoord);

	if (ALPHA_MASK) {
		if (color.a < ALPHA_MASK_CUTOFF) {
			discard;
		}
	}

	vec3 T = inTangent.xyz;
	vec3 N = inNormal;
	vec3 B = inBitangent;
	TBN = mat3(T, B, N);

	vec3 localNormal = 2.f * texture(samplerNormalMap, inTexcoord).rgb - 1.f;
	vec3 normal = normalize(TBN * localNormal);

	vec3 diffuse = calculate_diffuse(color.rgb, normal);
	vec3 specular = calculate_specular(inViewDir, inLightDir, normal);

	// outFragColor = vec4(inTangent.xyz, color.a);
	outFragColor = vec4(diffuse, color.a);

}