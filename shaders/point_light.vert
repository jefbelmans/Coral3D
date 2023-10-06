#version 450

const vec2 OFFSETS[6] = vec2[]
(
    vec2(-1.f, -1.f),
    vec2(-1.f, 1.f),
    vec2(1.f, -1.f),
    vec2(1.f, -1.f),
    vec2(-1.f, 1.f),
    vec2(1.f, 1.f)
);

layout (location = 0) out vec2 frag_offset;

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
    frag_offset = OFFSETS[gl_VertexIndex];
    vec3 cam_world_right = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
    vec3 cam_world_up = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

    vec3 world_pos = push.position.xyz
    + push.position.w * frag_offset.x * cam_world_right
    + push.position.w * frag_offset.y * cam_world_up;

    gl_Position = ubo.viewProjection * vec4(world_pos, 1.f);
}
