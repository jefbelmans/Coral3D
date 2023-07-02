#version 460

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;

layout(set = 0, binding = 0) uniform CameraBuffer
{
	mat4 view;
	mat4 projection;
	mat4 viewProj;
	mat4 viewInv;
} cameraData;

struct ObjectData
{
	mat4 world;
};

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer
{
	ObjectData objects[];
} objectBuffer;

void main()
{
	mat4 wvp = cameraData.viewProj * objectBuffer.objects[gl_InstanceIndex].world;
    gl_Position = wvp * vec4(vPosition, 1.0);
    outColor = vec3(0.1f, 0.8f, 0.1f);
	texCoord = vTexCoord;
}