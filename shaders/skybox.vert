#version 450

const vec3 VERTS[36] = vec3[]
(
vec3(-1.0f,  1.0f, -1.0f),
vec3(-1.0f, -1.0f, -1.0f),
vec3(1.0f, -1.0f, -1.0f),
vec3(1.0f, -1.0f, -1.0f),
vec3(1.0f,  1.0f, -1.0f),
vec3(-1.0f,  1.0f, -1.0f),

vec3(-1.0f, -1.0f,  1.0f),
vec3(-1.0f, -1.0f, -1.0f),
vec3(-1.0f,  1.0f, -1.0f),
vec3(-1.0f,  1.0f, -1.0f),
vec3(-1.0f,  1.0f,  1.0f),
vec3(-1.0f, -1.0f,  1.0f),

vec3(1.0f, -1.0f, -1.0f),
vec3(1.0f, -1.0f,  1.0f),
vec3(1.0f,  1.0f,  1.0f),
vec3(1.0f,  1.0f,  1.0f),
vec3(1.0f,  1.0f, -1.0f),
vec3(1.0f, -1.0f, -1.0f),

vec3(-1.0f, -1.0f,  1.0f),
vec3(-1.0f,  1.0f,  1.0f),
vec3(1.0f,  1.0f,  1.0f),
vec3(1.0f,  1.0f,  1.0f),
vec3(1.0f, -1.0f,  1.0f),
vec3(-1.0f, -1.0f,  1.0f),

vec3(-1.0f,  1.0f, -1.0f),
vec3(1.0f,  1.0f, -1.0f),
vec3(1.0f,  1.0f,  1.0f),
vec3(1.0f,  1.0f,  1.0f),
vec3(-1.0f,  1.0f,  1.0f),
vec3(-1.0f,  1.0f, -1.0f),

vec3(-1.0f, -1.0f, -1.0f),
vec3(-1.0f, -1.0f,  1.0f),
vec3(1.0f, -1.0f, -1.0f),
vec3(1.0f, -1.0f, -1.0f),
vec3(-1.0f, -1.0f,  1.0f),
vec3(1.0f, -1.0f,  1.0f)
);

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

layout(location = 0) out vec3 outTexCoord;

void main()
{
    outTexCoord = VERTS[gl_VertexIndex];
    gl_Position = ubo.viewProjection * vec4(outTexCoord + ubo.viewInverse[3].xyz, 1.f) ;
    outTexCoord.xy *= -1.f;
}