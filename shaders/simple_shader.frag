#version 450

layout (set = 1, binding = 0) uniform sampler2D samplerColorMap;
layout (set = 1, binding = 1) uniform sampler2D samplerNormalMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec4 inTangent;
layout (location = 2) in vec2 inTexcoord;
layout (location = 3) in vec3 inViewDir;
layout (location = 4) in vec3 inLightDir;

layout (location = 0) out vec4 outFragColor;

layout (constant_id = 0) const bool ALPHA_MASK = false;
layout (constant_id = 1) const float ALPHA_MASK_CUTOFF = 0.0f;

vec3 calculate_diffuse(vec3 color, vec3 normal)
{
	float diffuse_strength = dot(normal, -inLightDir);

	// HALF-LAMBERT
//	diffuse_strength = diffuse_strength * 0.5f + 0.5f;
//	diffuse_strength = clamp(diffuse_strength, 0.f, 1.f);

	return color * diffuse_strength;
}

void main()
{
	vec4 color = texture(samplerColorMap, inTexcoord);
	vec3 normal = texture(samplerNormalMap, inTexcoord).xyz;

	if (ALPHA_MASK) {
		if (color.a < ALPHA_MASK_CUTOFF) {
			discard;
		}
	}

	vec3 N = normalize(inNormal);
	vec3 T = normalize(inTangent.xyz);
	vec3 B = normalize(cross(T, N) * inTangent.w);
	mat3 TBN = mat3(T, B, N);
	N = normalize(normal * 2.0 - 1.0) * TBN;

	const float ambient = 0.01f;
	vec3 L = normalize(inLightDir);
	vec3 V = normalize(inViewDir);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), ambient).rrr;
	float specular = pow(max(dot(R, V), 0.0), 32.0);
	outFragColor = vec4(color.rgb, color.a);
}