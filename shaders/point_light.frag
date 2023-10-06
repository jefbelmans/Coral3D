#version 450

layout(location = 0) in vec2 frag_offset;
layout(location = 0) out vec4 out_color;

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

layout(push_constant) uniform Push
{
    vec4 position; // w is radius
    vec4 color; // w is intensity
} push;

void main()
{
    float dist = sqrt(dot(frag_offset, frag_offset));
    if(dist >= 1.f)
        discard;

    out_color = vec4(push.color.xyz, 1.f);
}