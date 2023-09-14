#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragPosWorld;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec3 fragTangent;
layout (location = 3) out vec3 fragBitangent;
layout (location = 4) out vec2 fragUV;
layout (location = 5) out mat3 fragTBN;

layout (set = 0, binding = 0) uniform GlobalUBO
{
	// MATRICES
	mat4 view;
	mat4 view_inverse;
    mat4 view_projection;

	// GLOBAL LIGHT
	vec4 global_light_direction;
	vec4 ambient_light_color;
} ubo;

layout (push_constant) uniform Push
{
	mat4 world_matrix;
	mat4 normal_matrix;
} push;

void main() 
{
	vec4 worldPos = push.world_matrix * vec4(position, 1.0f);
	gl_Position = ubo.view_projection * worldPos;

	fragPosWorld = worldPos.xyz;
	fragUV = uv;

	// NORMALS
	fragNormal = normalize(push.normal_matrix * vec4(normal, 0.f)).xyz;
	fragTangent = normalize(push.normal_matrix * vec4(tangent, 0.f)).xyz;
	fragBitangent = normalize(cross(normal, tangent));
	fragTBN = mat3(fragTangent, fragBitangent, fragNormal);
}