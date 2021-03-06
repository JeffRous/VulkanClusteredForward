//
// cluste_culling_ispc_avx2.h
// (Header automatically generated by the ispc compiler.)
// DO NOT EDIT THIS FILE.
//

#pragma once
#include <stdint.h>



#ifdef __cplusplus
namespace ispc { /* namespace */
#endif // __cplusplus
///////////////////////////////////////////////////////////////////////////
// Vector types with external visibility from ispc code
///////////////////////////////////////////////////////////////////////////

#ifndef __ISPC_VECTOR_float16__
#define __ISPC_VECTOR_float16__
#ifdef _MSC_VER
__declspec( align(64) ) struct float16 { float v[16]; };
#else
struct float16 { float v[16]; } __attribute__ ((aligned(64)));
#endif
#endif

#ifndef __ISPC_VECTOR_int32_t4__
#define __ISPC_VECTOR_int32_t4__
#ifdef _MSC_VER
__declspec( align(16) ) struct int32_t4 { int32_t v[4]; };
#else
struct int32_t4 { int32_t v[4]; } __attribute__ ((aligned(16)));
#endif
#endif

#ifndef __ISPC_VECTOR_int32_t2__
#define __ISPC_VECTOR_int32_t2__
#ifdef _MSC_VER
__declspec( align(16) ) struct int32_t2 { int32_t v[2]; };
#else
struct int32_t2 { int32_t v[2]; } __attribute__ ((aligned(16)));
#endif
#endif

#ifndef __ISPC_VECTOR_float3__
#define __ISPC_VECTOR_float3__
#ifdef _MSC_VER
__declspec( align(16) ) struct float3 { float v[3]; };
#else
struct float3 { float v[3]; } __attribute__ ((aligned(16)));
#endif
#endif

#ifndef __ISPC_VECTOR_float2__
#define __ISPC_VECTOR_float2__
#ifdef _MSC_VER
__declspec( align(16) ) struct float2 { float v[2]; };
#else
struct float2 { float v[2]; } __attribute__ ((aligned(16)));
#endif
#endif



#ifndef __ISPC_ALIGN__
#if defined(__clang__) || !defined(_MSC_VER)
// Clang, GCC, ICC
#define __ISPC_ALIGN__(s) __attribute__((aligned(s)))
#define __ISPC_ALIGNED_STRUCT__(s) struct __ISPC_ALIGN__(s)
#else
// Visual Studio
#define __ISPC_ALIGN__(s) __declspec(align(s))
#define __ISPC_ALIGNED_STRUCT__(s) __ISPC_ALIGN__(s) struct
#endif
#endif

#ifndef __ISPC_STRUCT_ScreenToViewISPC__
#define __ISPC_STRUCT_ScreenToViewISPC__
struct ScreenToViewISPC {
    float16  inverseProjection;
    float16  viewMatrix;
    int32_t4  tileSizes;
    int32_t2  screenDimensions;
    float zNear;
    float zFar;
};
#endif

#ifndef __ISPC_STRUCT_PointLightDataISPC__
#define __ISPC_STRUCT_PointLightDataISPC__
struct PointLightDataISPC {
    float3  pos;
    float radius;
    float3  color;
    uint32_t enabled;
    float ambient_intensity;
    float diffuse_intensity;
    float specular_intensity;
    float attenuation_constant;
    float attenuation_linear;
    float attenuation_exp;
    float2  padding;
};
#endif

#ifndef __ISPC_STRUCT_LightGrid__
#define __ISPC_STRUCT_LightGrid__
struct LightGrid {
    uint32_t offset;
    uint32_t count;
};
#endif


///////////////////////////////////////////////////////////////////////////
// Functions exported from ispc code
///////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
extern "C" {
#endif // __cplusplus
    extern void cluste_culling_ispc(int32_t xSize, int32_t ySize, int32_t zSize, struct ScreenToViewISPC &screenToView, struct PointLightDataISPC * pointLights, int32_t lightCount, struct LightGrid * lightGrids, uint32_t * globalLightIndexList);
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
} /* end extern C */
#endif // __cplusplus


#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus
