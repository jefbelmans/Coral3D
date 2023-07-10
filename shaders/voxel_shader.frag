#version 460

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in vec3 fragNormal;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUBO
{
	mat4 viewProjection;
	vec4 globalLightDirection;
} ubo;

layout (set = 0, binding = 1) uniform sampler2D texSampler;

layout (push_constant) uniform Push
{
   mat4 world;
} push;

void main()
{
	outColor = texture(texSampler, fragTexCoord);
}