#version 460

layout (location = 0) in vec3 posistion;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

layout (location = 0) out vec2 fragTexCoord;
layout (location = 1) out vec3 fragNormal;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	mat4 viewProjection;
	vec4 globalLightDirection;
} ubo;

layout (push_constant) uniform Push
{
   vec3 position;
} push;

void main()
{
	gl_Position = ubo.viewProjection * vec4(posistion + push.position, 1.f);

	fragTexCoord = texCoord;
	fragNormal = normal;
}