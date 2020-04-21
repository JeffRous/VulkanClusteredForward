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

struct ScreenToView{
    mat4 inverseProjection;
    mat4 viewMatrix;
    uvec4 tileSizes;
    uvec2 screenDimensions;
    float zNear;
    float zFar;
};

struct PointLightData{
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
static vec4 clipToView(vec4 clip, uniform ScreenToView& screenToView);
static vec4 screen2View(vec4 screen, uniform ScreenToView& screenToView);
static vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance);
static inline float dot(vec3 a, vec3 b);
static inline vec3 vcross(vec3 v0, vec3 v1);
static inline vec2 toVec2(float x, float y);
static inline vec3 toVec3(float v);
static inline vec3 toVec3(float x, float y, float z);
static inline vec3 toVec3(vec4 v);
static inline vec4 toVec4(vec2 v, float z, float w);
static inline vec4 toVec4(vec3 v, float w);
static VolumeTileAABB toVolumeTileAABB(vec4 minP, vec4 maxP);
static vec4 apply(mat4 m, vec4 v);
static vec3 min(vec3 v1, vec3 v2);
static vec3 max(vec3 v1, vec3 v2);
static bool testSphereAABB(uniform ScreenToView& screenToView, vec3 pos, float radius, vec3 minPoint, vec3 maxPoint);
static float sqDistPointAABB(vec3 point, vec3 minPoint, vec3 maxPoint);

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

vec4 clipToView(vec4 clip, uniform ScreenToView& screenToView){
    //View space transform
    vec4 view = apply(screenToView.inverseProjection, clip);

    //Perspective projection
    view = view / view.w;
    
    return view;
}

vec4 screen2View(vec4 screen, uniform ScreenToView& screenToView)
{
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

inline vec3 toVec3(float v)
{
    vec3 ret;
    ret.x = v;
    ret.y = v;
    ret.z = v;
    return ret;
}

inline vec3 toVec3(float x, float y, float z)
{
    vec3 ret;
    ret.x = x;
    ret.y = y;
    ret.z = z;
    return ret;
}

inline vec3 toVec3(vec4 v)
{
    vec3 ret;
    ret.x = v.x;
    ret.y = v.y;
    ret.z = v.z;
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

inline vec4 toVec4(vec3 v, float w)
{
    vec4 ret;
    ret.x = v.x;
    ret.y = v.y;
    ret.z = v.z;
    ret.w = w;
    return ret;
}

VolumeTileAABB toVolumeTileAABB(vec4 minP, vec4 maxP)
{
    VolumeTileAABB ret;
    ret.minPoint = minP;
    ret.maxPoint = maxP;
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

vec3 min(vec3 v1, vec3 v2)
{
    vec3 ret;
    ret.x = v1.x > v2.x ? v2.x : v1.x;
    ret.y = v1.y > v2.y ? v2.y : v1.y;
    ret.z = v1.z > v2.z ? v2.z : v1.z;
    return ret;
}

vec3 max(vec3 v1, vec3 v2)
{
    vec3 ret;
    ret.x = v1.x > v2.x ? v1.x : v2.x;
    ret.y = v1.y > v2.y ? v1.y : v2.y;
    ret.z = v1.z > v2.z ? v1.z : v2.z;
    return ret;
}

bool testSphereAABB(uniform ScreenToView& screenToView, vec3 pos, float radius, vec3 minPoint, vec3 maxPoint)
{
    vec3 center  = toVec3(apply(screenToView.viewMatrix, toVec4(pos, 1.0f)));
    float squaredDistance = sqDistPointAABB(center, minPoint, maxPoint);

    bool ret = (squaredDistance <= (radius * radius));
    return ret;
}

float sqDistPointAABB(vec3 point, vec3 minPoint, vec3 maxPoint)
{
    float sqDist = 0.0;
    for(int i = 0; i < 3; ++i){
        float v = point[i];
        if(v < maxPoint[i]){
            sqDist += (maxPoint[i] - v) * (maxPoint[i] - v);
        }
        if(v > maxPoint[i]){
            sqDist += (v - maxPoint[i]) * (v - maxPoint[i]);
        }
    }

    return sqDist;
}

/// 16, 9, 24
export void cluste_culling_ispc (uniform int xSize, uniform int ySize, uniform int zSize, uniform ScreenToView& screenToView, uniform PointLightData pointLights[], uniform int lightCount, uniform LightGrid lightGrids[], uniform uint globalLightIndexList[])
{
    uniform int32 globalIndexCount = 0;

    //Eye position is zero in view space
    const vec3 eyePos = toVec3(0.0);
    float zNear = screenToView.zNear;
    float zFar = screenToView.zFar;

    //Per Tile variables
    float tileSizePx = screenToView.tileSizes[3];
    for(int x = 0; x < xSize; x++)
    {
        for(int y = 0; y < ySize; y++)
        {
            for(int z = 0; z < zSize; z++)
            {
                uint tileIndex = x + y * xSize + z * xSize * ySize;

                //Calculating the min and max point in screen space
                vec4 maxPoint_sS = toVec4(toVec2(x + 1, y + 1) * tileSizePx, -1.0, 1.0); // Top Right
                vec4 minPoint_sS = toVec4(toVec2(x, y) * tileSizePx, -1.0, 1.0); // Bottom left

                //Pass min and max to view space
                vec3 maxPoint_vS = toVec3(screen2View(maxPoint_sS, screenToView));
                vec3 minPoint_vS = toVec3(screen2View(minPoint_sS, screenToView));

                //Near and far values of the cluster in view space
                float tileNear  = -zNear * pow(zFar/ zNear, (float)z/zSize);
                float tileFar   = -zNear * pow(zFar/ zNear, (float)(z + 1) /zSize);

                //Finding the 4 intersection points made from the maxPoint to the cluster near/far plane
                vec3 minPointNear = lineIntersectionToZPlane(eyePos, minPoint_vS, tileNear );
                vec3 minPointFar  = lineIntersectionToZPlane(eyePos, minPoint_vS, tileFar );
                vec3 maxPointNear = lineIntersectionToZPlane(eyePos, maxPoint_vS, tileNear );
                vec3 maxPointFar  = lineIntersectionToZPlane(eyePos, maxPoint_vS, tileFar );

                vec3 minPointAABB = min(min(minPointNear, minPointFar),min(maxPointNear, maxPointFar));
                vec3 maxPointAABB = max(max(minPointNear, minPointFar),max(maxPointNear, maxPointFar));

                uint visibleLightIndices[100];
                uint visibleLightCount = 0;

                for(int light = 0; light < lightCount; light++)
                {
                    if(pointLights[light].enabled  == 1)
                    {
                        if( testSphereAABB(screenToView, pointLights[light].pos, pointLights[light].radius, minPointAABB, maxPointAABB) )
                        {
                            visibleLightIndices[visibleLightCount] = light;
                            visibleLightCount += 1;
                        }
                    }
                }

                uint offset = atomic_add_global(&globalIndexCount, visibleLightCount);

                for(uint i = 0; i < visibleLightCount; ++i)
                {
                    globalLightIndexList[offset + i] = visibleLightIndices[i];
                }

                lightGrids[tileIndex].offset = offset;
                lightGrids[tileIndex].count = visibleLightCount;
            }
        }
    }
}