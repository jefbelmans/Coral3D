#version 450

layout (location = 0) in vec3 frag_pos_world;
layout (location = 1) in vec3 frag_normal;
layout (location = 2) in vec3 frag_tangent;
layout (location = 3) in vec3 frag_bitangent;
layout (location = 4) in vec2 frag_tex_coord;
layout (location = 5) in mat3 frag_TBN;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform GlobalUBO
{
	// MATRICES
	mat4 view;
	mat4 view_inverse;
	mat4 view_projection;

	// GLOBAL LIGHT
	vec4 global_light_direction;
	vec4 ambient_light_color;
} global_ubo;

layout (set = 1, binding = 0) uniform sampler samp;
// TEXTURE ARRAY LAYOUT:
// 0: Diffuse
// 1: Specular
// 2: Normal
// 3: Opacity
// 4: Reserved
// 5: Reserved
// 6: Reserved
// 7: Reserved
layout (set = 1, binding = 1) uniform texture2D textures[8];
layout (set = 1, binding = 2) uniform MaterialUBO
{
	bool use_diffuse_map;
	vec3 diffuse_color;

	bool use_specular_map;
	vec3 specular_color;
	float shininess;

	bool use_bump_map;

	bool use_opacity_map;
	float opacity_value;
} material_ubo;

layout (push_constant) uniform Push
{
	mat4 world_matrix;
	mat4 normal_matrix;
} push;

// HELPERS
vec3 calculate_normal(vec3 tangent, vec3 normal, vec2 tex_coord)
{
	if(!material_ubo.use_bump_map) return normal;
	vec3 new_normal = normal;

	// NORMALIZE
	normal = normalize(normal);
	tangent = normalize(tangent);

	// BINORMAL
	vec3 binormal = normalize(cross(tangent, normal));

	// LOCAL AXIS
	mat3 local_axis = mat3(tangent, binormal, normal);

	// SAMPLED NORMAL
	vec3 sampled_normal = texture(sampler2D(textures[2], samp), tex_coord).xyz;
	sampled_normal = normalize(sampled_normal * 2.0f - 1.f);
	new_normal = sampled_normal * local_axis;

	return new_normal;
}

vec3 calculate_normal()
{
	vec3 normal = texture(sampler2D(textures[2], samp), frag_tex_coord).xyz;
	normal = normalize(normal * 2.0f - 1.0f);
	normal = normalize(frag_TBN * normal);

	return normal;
}

vec3 calculate_diffuse(vec3 color, vec3 normal)
{
	float diffuse_strength = dot(normal, -global_ubo.global_light_direction.xyz);

	// HALF-LAMBERT
	diffuse_strength = diffuse_strength * 0.5f + 0.5f;
	diffuse_strength = clamp(diffuse_strength, 0.f, 1.f);

	return color * diffuse_strength;
}

vec3 calculate_specular_blinn(vec3 view_dir, vec3 normal, vec2 tex_coord)
{
	vec3 half_angle = -normalize(view_dir + global_ubo.global_light_direction.xyz);
	float specular_strength = clamp(dot(half_angle, normal), 0.f, 1.f);

	float exp = material_ubo.shininess * 4;
	if(material_ubo.use_specular_map)
		exp *= texture(sampler2D(textures[1], samp), tex_coord).r;

	float specularity = pow(specular_strength, exp);

	return material_ubo.specular_color * specularity;
}

float calculate_opacity(vec2 tex_coord)
{
	float opacity = material_ubo.opacity_value;
	if(material_ubo.use_opacity_map)
		opacity = texture(sampler2D(textures[3], samp), tex_coord).r;
	return opacity;
}

void main()
{
	// OPACITY
	float opacity = calculate_opacity(frag_tex_coord);
	if(opacity < 0.1) discard; // early discard

	// VIEW DIRECTION
	vec3 view_direction = frag_pos_world - global_ubo.view_inverse[3].xyz;

	vec3 normal = calculate_normal(frag_tangent, frag_normal, frag_tex_coord);

	// DIFFUSE
	vec3 color = material_ubo.diffuse_color;
	if(material_ubo.use_diffuse_map)
		color = texture(sampler2D(textures[0], samp), frag_tex_coord).xyz;

	vec3 diffuse_color = calculate_diffuse(color, normal);

	// SPECULAR
	vec3 specular_color = calculate_specular_blinn(view_direction, normal, frag_tex_coord);

	// AMBIENT
	vec3 ambient = global_ubo.ambient_light_color.xyz * global_ubo.ambient_light_color.w;

	out_color = vec4(diffuse_color + ambient, 1.f);
}