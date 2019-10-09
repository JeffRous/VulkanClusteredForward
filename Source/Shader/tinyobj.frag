#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHT_NUM 16
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
	vec3 color;
    float ambient_intensity;
	float diffuse_intensity;
	float specular_intensity;
    float attenuation_constant;
	float attenuation_linear;
	float attenuation_exp;
} pointLight[MAX_LIGHT_NUM];

layout(binding = 3) uniform sampler2D albedoSampler;
layout(binding = 4) uniform sampler2D normalSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 tanViewPos;
layout(location = 4) in vec3 tanFragPos;
layout(location = 5) in vec3 tanLightPos[16];

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0,0,0,1);
    for(int i = 0; i < MAX_LIGHT_NUM; i++)
    {
        // diffuse
        vec3 albedo;
        if (material.has_albedo_map > 0)
        {
            albedo = texture(albedoSampler, fragTexCoord.xy).rgb;
        }
        else
        {
            albedo = vec3(1.0);
        }
        // ambient
        vec3 ambient = pointLight[i].ambient_intensity * albedo * pointLight[i].color;
        // normal
        vec3 normal;
        if (material.has_normal_map > 0)
        {
            normal = texture(normalSampler, fragTexCoord.xy).rgb;
            normal = normalize(normal * 2.0 - 1.0);
        }
        else
        {
            normal = vec3(0, 0, 1);
        }
        // diffuse
        vec3 lightDir = normalize(tanLightPos[i] - tanFragPos);
        float lambertian = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = pointLight[i].diffuse_intensity * albedo * lambertian * pointLight[i].color;
        // specular
        vec3 viewDir  = normalize(tanViewPos - tanFragPos);
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, normal), 0.0);
        float spec = pow(specAngle, 32);
        vec3 specular = pointLight[i].specular_intensity * spec * pointLight[i].color;
        // attenuation
        float distance    = length(pointLight[i].pos - fragPos);
        ///float attenuation = 1.0 / (pointLight[i].attenuation_constant + pointLight[i].attenuation_linear * distance + pointLight[i].attenuation_exp * (distance * distance)); 
        float attenuation = pow(smoothstep(pointLight[i].radius, 0, distance), 2);
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        // final color
        outColor.xyz += vec3(ambient + diffuse + specular);
    }
}