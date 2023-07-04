#version 450

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push
{
	mat3 transform;
	vec3 offset;
	vec3 color;
} push;

void main()
{
  outColor = vec4(push.color, 1.0f);
}