#define MAX_LIGHT_NUM 16

typedef int<2> uvec2;
typedef int<3> uvec3;
typedef int<4> uvec4;
typedef float<2> vec2;
typedef float<3> vec3;
typedef float<4> vec4;
typedef float<16> mat4;

//ssbo initialization
struct VolumeTileAABB{
    vec4 minPoint;
    vec4 maxPoint;
};

struct ScreenToViewISPC{
    mat4 inverseProjection;
    mat4 viewMatrix;
    uvec4 tileSizes;
    uvec2 screenDimensions;
    float zNear;
    float zFar;
};

struct PointLightDataISPC{
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

inline vec2 toVec2(float x, float y)
{
    vec2 ret;
    ret.x = x;
    ret.y = y;
    return ret;
}

inline uniform vec2 toVec2(uniform float x, uniform float y)
{
    uniform vec2 ret;
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

inline vec3 toVec3(vec4& v)
{
    vec3 ret;
    ret.x = v.x;
    ret.y = v.y;
    ret.z = v.z;
    return ret;
}

inline uniform vec3 toVec3(uniform vec4& v)
{
    uniform vec3 ret;
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

inline uniform vec4 toVec4(uniform vec2& v, uniform float z, uniform float w)
{
    uniform vec4 ret;
    ret.x = v.x;
    ret.y = v.y;
    ret.z = z;
    ret.w = w;
    return ret;
}

inline vec4 toVec4(vec3& v, float w)
{
    vec4 ret;
    ret.x = v.x;
    ret.y = v.y;
    ret.z = v.z;
    ret.w = w;
    return ret;
}

inline uniform vec4 toVec4(uniform vec3& v, uniform float w)
{
    uniform vec4 ret;
    ret.x = v.x;
    ret.y = v.y;
    ret.z = v.z;
    ret.w = w;
    return ret;
}

inline VolumeTileAABB toVolumeTileAABB(vec4& minP, vec4& maxP)
{
    VolumeTileAABB ret;
    ret.minPoint = minP;
    ret.maxPoint = maxP;
    return ret;
}

inline uniform float dot(const uniform vec3& a, const uniform vec3& b) 
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 vcross(vec3& v0, vec3& v1) 
{
    vec3 ret;
    ret.x = v0.y * v1.z - v0.z * v1.y;
    ret.y = v0.z * v1.x - v0.x * v1.z;
    ret.z = v0.x * v1.y - v0.y * v1.x;
    return ret;
}

inline uniform vec4 apply(uniform mat4& m, uniform vec4& v)
{
    uniform vec4 ret;
    foreach(col = 0 ... 4)// (uniform int col = 0; col < 4; col++) 
    {
        float sum = 0;
        for (uniform int row = 0; row < 4; row++) {
            sum += v[row] * m[row * 4 + col];
        }
        ret[col] = sum;
    }
    return ret;
}

inline vec4 apply(uniform mat4& m, vec4& v)
{
    vec4 ret;
    for (uniform int col = 0; col < 4; col++) 
    {
        float sum = 0;
        for (uniform int row = 0; row < 4; row++) {
            sum += v[row] * m[row * 4 + col];
        }
        ret[col] = sum;
    }
    return ret;
}

inline uniform float apply_parallel( uniform mat4& m, uniform vec3& v )
{
	return m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
}

inline vec3 min(vec3& v1, vec3& v2)
{
    vec3 ret;
    ret.x = v1.x > v2.x ? v2.x : v1.x;
    ret.y = v1.y > v2.y ? v2.y : v1.y;
    ret.z = v1.z > v2.z ? v2.z : v1.z;
    return ret;
}

inline vec3 max(vec3& v1, vec3& v2)
{
    vec3 ret;
    ret.x = v1.x > v2.x ? v1.x : v2.x;
    ret.y = v1.y > v2.y ? v1.y : v2.y;
    ret.z = v1.z > v2.z ? v1.z : v2.z;
    return ret;
}

inline vec3 lineIntersectionToZPlane(const uniform vec3& A, const uniform vec3& B, const float zDistance){
    //Because this is a Z based normal this is fixed
    static const uniform vec3 normal = {0.0, 0.0, 1.0};

    uniform vec3 ab =  B - A;

    //Computing the intersection length for the line and the plane
    float t = (zDistance - dot(normal, A)) / dot(normal, ab);

    //Computing the actual xyz position of the point along the line
    vec3 result = A + t * ab;

    return result;
}

inline uniform vec4 clipToView(uniform vec4& clip, uniform ScreenToViewISPC& screenToView){
    //View space transform
    uniform vec4 view = apply(screenToView.inverseProjection, clip);

    //Perspective projection
    view = view / view.w;
    
    return view;
}

inline uniform vec4 screen2View(uniform vec4& screen, uniform ScreenToViewISPC& screenToView)
{
    //Convert to NDC
    uniform vec2 texCoord = screen.xy / screenToView.screenDimensions.xy;

    //Convert to clipSpace
    // vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    uniform vec4 clip = toVec4(toVec2(texCoord.x, texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    //Not sure which of the two it is just yet

    return clipToView(clip, screenToView);
}

inline float sqDistPointAABB(uniform vec3& point, vec3& minPoint, vec3& maxPoint)
{
    float sqDist = 0.0;
    for(uniform int i = 0; i < 3; ++i){
        uniform float v = point[i];
        if(v < minPoint[i]){
            sqDist += (minPoint[i] - v) * (minPoint[i] - v);
        }
        if(v > maxPoint[i]){
            sqDist += (v - maxPoint[i]) * (v - maxPoint[i]);
        }
    }

    return sqDist;
}

inline bool testSphereAABB(uniform ScreenToViewISPC& screenToView, uniform vec3& pos, uniform float& radius, vec3 minPoint, vec3 maxPoint)
{
    uniform vec3 center  = toVec3(apply(screenToView.viewMatrix, toVec4(pos, 1.0f)));
	//uniform float center_ = apply_parallel(screenToView.viewMatrix, pos);
	//vec3 center;
	//center.x = broadcast(center_, 0);
	//center.y = broadcast(center_, 1);
	//center.z = broadcast(center_, 2);
	float squaredDistance = sqDistPointAABB(center, minPoint, maxPoint);
	bool ret = (squaredDistance <= (radius * radius));
    return ret;
}

/// 16, 9, 24
export void cluste_culling_ispc (const uniform int xSize, const uniform int ySize, uniform int zSize, uniform ScreenToViewISPC& screenToView, uniform PointLightDataISPC pointLights[], const uniform int lightCount, uniform LightGrid lightGrids[], uniform uint globalLightIndexList[])
{
    uniform int32 globalIndexCount = 0;

    //Eye position is zero in view space
    const uniform vec3 eyePos = { 0.0, 0.0, 0.0 };
    const uniform float zNear = screenToView.zNear;
    const uniform float zFar = screenToView.zFar;

    //Per Tile variables
    const uniform float tileSizePx = screenToView.tileSizes[3];
    for(uniform int x = 0; x < xSize; x++)
    {
        for(uniform int y = 0; y < ySize; y++)
        {
            //Calculating the min and max point in screen space
            uniform vec4 maxPoint_sS = toVec4(toVec2(x + 1, y + 1) * tileSizePx, -1.0, 1.0); // Top Right
            uniform vec4 minPoint_sS = toVec4(toVec2(x, y) * tileSizePx, -1.0, 1.0); // Bottom left

            //Pass min and max to view space
            uniform vec3 maxPoint_vS = toVec3(screen2View(maxPoint_sS, screenToView));
            uniform vec3 minPoint_vS = toVec3(screen2View(minPoint_sS, screenToView));

            foreach(z = 0 ... zSize)
            {
                varying uint tileIndex = x + y * xSize + z * xSize * ySize;

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

                varying uint visibleLightIndices[100];
                varying uint visibleLightCount = 0;

                for(uniform int light = 0; light < lightCount; light++)
                {
                    if(pointLights[light].enabled  == 1)
                    {
                        if( testSphereAABB(screenToView, pointLights[light].pos, pointLights[light].radius, minPointAABB, maxPointAABB) )
                        {
                            #pragma ignore warning(perf)
                            visibleLightIndices[visibleLightCount] = light;
                            visibleLightCount += 1;
                        }
                    }
                }

                varying uint offset = atomic_add_local(&globalIndexCount, visibleLightCount);

                for(uniform uint i = 0; i < visibleLightCount; ++i)
                {
                    #pragma ignore warning(perf)
                    globalLightIndexList[offset + i] = visibleLightIndices[i];
                }

                #pragma ignore warning(perf)
                lightGrids[tileIndex].offset = offset;
                #pragma ignore warning(perf)
                lightGrids[tileIndex].count = visibleLightCount;
            }
        }
    }
}