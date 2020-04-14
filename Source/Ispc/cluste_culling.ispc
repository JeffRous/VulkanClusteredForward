typedef int<2> uvec2;
typedef int<3> uvec3;
typedef int<4> uvec4;
typedef float<2> vec2;
typedef float<3> vec3;
typedef float<4> vec4;
typedef float<16> mat4;
typedef unsigned int uint;

//ssbo initialization
struct VolumeTileAABB{
    vec4 minPoint;
    vec4 maxPoint;
};

struct ClusterAABB{
    VolumeTileAABB cluster[];
};

struct ScreenToView{
    mat4 inverseProjection;
    mat4 viewMatrix;
    uvec4 tileSizes;
    uvec2 screenDimensions;
    float zNear;
    float zFar;
};

struct PointLight{
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
};

struct LightGrid{
    uint offset;
    uint count;
};

//Function prototypes
vec4 clipToView(vec4 clip);
vec4 screen2View(vec4 screen);
vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance);

vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance){
    //Because this is a Z based normal this is fixed
    vec3 normal = vec3(0.0, 0.0, 1.0);

    vec3 ab =  B - A;

    //Computing the intersection length for the line and the plane
    float t = (zDistance - dot(normal, A)) / dot(normal, ab);

    //Computing the actual xyz position of the point along the line
    vec3 result = A + t * ab;

    return result;
}

vec4 clipToView(vec4 clip){
    //View space transform
    vec4 view = inverseProjection * clip;

    //Perspective projection
    view = view / view.w;
    
    return view;
}

vec4 screen2View(vec4 screen){
    //Convert to NDC
    vec2 texCoord = screen.xy / screenDimensions.xy;

    //Convert to clipSpace
    // vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    vec4 clip = vec4(vec2(texCoord.x, texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    //Not sure which of the two it is just yet

    return clipToView(clip);
}

export void cluste_culling_ispc (uniform int x, uniform int y, uniform int z, uniform PointLight pointLights[], uniform LightGrid lightGrids[], uniform uint globalLightIndexList[])
{
    
}