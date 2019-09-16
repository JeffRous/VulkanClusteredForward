#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 0) uniform TransformData {
    mat4 mvp;
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 proj_view;
    vec3 cam_pos;
} transform;

layout(std140, binding = 1) uniform MaterialData
{
    int has_albedo_map;
    int has_normal_map;
} material;

layout(std140, binding = 2) uniform PointLightData
{
    vec3 pos;
	float radius;
	vec3 intensity;
} pointLight;

layout(binding = 3) uniform sampler2D albedoSampler;
layout(binding = 4) uniform sampler2D normalSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(albedoSampler, fragTexCoord.xy);
}