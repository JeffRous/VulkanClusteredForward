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
static vec4 clipToView(vec4 clip);
static vec4 screen2View(vec4 screen);
static vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance);
static inline float dot(vec3 a, vec3 b);
static inline vec3 vcross(vec3 v0, vec3 v1);
static inline vec2 toVec2(float x, float y);
static inline vec4 toVec4(vec2 v, float z, float w);
static vec4 apply(mat4 m, vec4 v);

vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance){
    //Because this is a Z based normal this is fixed
    vec3 normal = {0.0, 0.0, 1.0};

    vec3 ab =  B - A;

    //Computing the intersection length for the line and the plane
    float t = (zDistance - dot(normal, A)) / dot(normal, ab);

    //Computing the actual xyz position of the point along the line
    vec3 result = A + t * ab;

    return result;
}

vec4 clipToView(vec4 clip, ScreenToView& screenToView){
    //View space transform
    vec4 view = apply(screenToView.inverseProjection, clip);

    //Perspective projection
    view = view / view.w;
    
    return view;
}

vec4 screen2View(vec4 screen, ScreenToView& screenToView){
    //Convert to NDC
    vec2 texCoord = screen.xy / screenToView.screenDimensions.xy;

    //Convert to clipSpace
    // vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    vec4 clip = toVec4(toVec2(texCoord.x, texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    //Not sure which of the two it is just yet

    return clipToView(clip, screenToView);
}

inline vec2 toVec2(float x, float y)
{
    vec2 ret;
    ret.x = x;
    ret.y = y;
    return ret;
}

inline vec4 toVec4(vec2 v, float z, float w)
{
    vec4 ret;
    ret.x = v.x;
    ret.y = v.y;
    ret.z = z;
    ret.w = w;
    return ret;
}

inline float dot(vec3 a, vec3 b) 
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 vcross(vec3 v0, vec3 v1) 
{
    vec3 ret;
    ret.x = v0.y * v1.z - v0.z * v1.y;
    ret.y = v0.z * v1.x - v0.x * v1.z;
    ret.z = v0.x * v1.y - v0.y * v1.x;
    return ret;
}

vec4 apply(mat4 m, vec4 v)
{
    vec4 ret;
    for (size_t row = 0; row < 4; row++) 
    {
        float sum = 0;
        for (int col = 0; col < 4; col++) {
            sum += v[col] * m[row*4 +col];
        }
        ret[row] = sum;
    }
    return ret;
}

export void cluste_culling_ispc (uniform int x, uniform int y, uniform int z, uniform ScreenToView& screenToView, uniform PointLight pointLights[], uniform LightGrid lightGrids[], uniform uint globalLightIndexList[])
{
    
}