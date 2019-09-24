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
	vec3 color;
    float ambient_intensity;
	float diffuse_intensity;
	float specular_intensity;
    float attenuation_constant;
	float attenuation_linear;
	float attenuation_exp;
} pointLight;

layout(binding = 3) uniform sampler2D albedoSampler;
layout(binding = 4) uniform sampler2D normalSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 tanLightPos;
layout(location = 4) in vec3 tanViewPos;
layout(location = 5) in vec3 tanFragPos;

layout(location = 0) out vec4 outColor;

void main() {
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
    vec3 ambient = pointLight.ambient_intensity * albedo * pointLight.color;
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
    vec3 lightDir = normalize(tanLightPos - tanFragPos);
    float lambertian = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = pointLight.diffuse_intensity * albedo * lambertian * pointLight.color;
    // specular
    vec3 viewDir  = normalize(tanViewPos - tanFragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normal), 0.0);
    float spec = pow(specAngle, 32);
    vec3 specular = pointLight.specular_intensity * spec * pointLight.color;
    // attenuation
    float distance    = length(pointLight.pos - fragPos);
    float attenuation = 1.0 / (pointLight.attenuation_constant + pointLight.attenuation_linear * distance + pointLight.attenuation_exp * (distance * distance)); 
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    // final color
    outColor = vec4(ambient + diffuse + specular, 1.0);
}