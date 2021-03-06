#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHT_NUM 16

struct LightGrid{
    uint offset;
    uint count;
};

layout (std140, binding = 0) uniform TransformData {
    mat4 mvp;
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 proj_view;
    vec3 cam_pos;
    bool isClusteShading;
    uvec4 tileSizes;
    float zNear;
    float zFar;
    float scale;
    float bias;
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
    uint enabled;
    float ambient_intensity;
	float diffuse_intensity;
	float specular_intensity;
    float attenuation_constant;
	float attenuation_linear;
	float attenuation_exp;
    vec2 padding;
} pointLight[MAX_LIGHT_NUM];

layout (std430, binding = 3) readonly buffer lightIndexSSBO{
    uint globalLightIndexList[];
};
layout (std430, binding = 4) readonly buffer lightGridSSBO{
    LightGrid lightGrid[];
};

layout(binding = 5) uniform sampler2D albedoSampler;
layout(binding = 6) uniform sampler2D normalSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 tanViewPos;
layout(location = 4) in vec3 tanFragPos;
layout(location = 5) in vec3 tanLightPos[16];

layout(location = 0) out vec4 outColor;

float linearDepth(float depthSample){
    float depthRange = 2.0 * depthSample - 1.0;
    float linear = 2.0 * transform.zNear * transform.zFar / (transform.zFar + transform.zNear - depthRange * (transform.zFar - transform.zNear));
    return linear;
}

vec3 lightingColor(uint i);

void main() {
    outColor = vec4(0,0,0,1);

    if(transform.isClusteShading)
    {
        uint zTile = uint(max(log2(linearDepth(gl_FragCoord.z)) * transform.scale + transform.bias, 0.0));
        uvec3 tiles = uvec3( uvec2( gl_FragCoord.xy / transform.tileSizes[3] ), zTile);
        uint tileIndex = tiles.x +
                        transform.tileSizes.x * tiles.y +
                        (transform.tileSizes.x * transform.tileSizes.y) * tiles.z;
        ///float color = float(zTile) / transform.tileSizes.z;
        ///outColor.xyz = vec3(color, color, color);
        ///return;

        uint offset = lightGrid[tileIndex].offset;
        uint visibleLightCount = lightGrid[tileIndex].count;
        for(int idx = 0; idx < visibleLightCount; idx++)
        {
            uint i = globalLightIndexList[offset + idx];

            // final color
            outColor.xyz += lightingColor(i);
        }
    }
    else
    {
        for(int i = 0; i < MAX_LIGHT_NUM; i++)
        {
            // final color
            outColor.xyz += lightingColor(i);
        }
    }
}

vec3 lightingColor(uint i)
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
    return vec3(ambient + diffuse + specular);
}