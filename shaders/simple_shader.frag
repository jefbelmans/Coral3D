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

vec3 calculate_diffuse(vec3 color)
{
	float diffuse_strength = dot(inNormal, -inLightDir);

	// HALF-LAMBERT
	diffuse_strength = diffuse_strength * 0.5f + 0.5f;
	diffuse_strength = clamp(diffuse_strength, 0.f, 1.f);

	return color * diffuse_strength;
}

void main()
{
	vec4 color = texture(samplerColorMap, inTexcoord);

	if (ALPHA_MASK) {
		if (color.a < ALPHA_MASK_CUTOFF) {
			discard;
		}
	}

	vec3 diffuse_color = calculate_diffuse(color.rgb);
	outFragColor = vec4(diffuse_color, color.a);
}