#version 450

struct PointLight
{
    vec4 position; // w is radius
    vec4 color; // w is intensity
};

layout (set = 0, binding = 0) uniform GlobalUBO
{
// MATRICES
    mat4 view;
    mat4 viewInverse;
    mat4 viewProjection;

// LIGHTING
    vec4 globalLightDirection;
    vec4 ambientLighting;

// POINT LIGHTS
    PointLight pointLights[8];
    int numLights;
} ubo;

// SAMPLERS
layout (set = 0, binding = 1) uniform samplerCube samplerCubeMap;

layout(location = 0) in vec3 inTexCoord;
layout(location = 0) out vec4 outFragColor;


void main()
{
    outFragColor = texture(samplerCubeMap, inTexCoord);
}