// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#if !defined(MATERIAL_QUERY_H)
#define MATERIAL_QUERY_H

#include "LightingAlgorithm.h"
#include "../gbuffer.h"

cbuffer MaterialOverride : register(b9)
{
    float MO_Metallic;
    float MO_Roughness;
    float MO_Specular;
    float MO_Specular2;
    float MO_Material;
    float MO_Material2;

    float MO_DiffuseScale;
    float MO_ReflectionsScale;
    float MO_ReflectionsBoost;
    float MO_Specular0Scale;
    float MO_Specular1Scale;
}

static const bool UseMaterialOverride = false;

float Material_GetMetal(GBufferValues gbuffer)
{
    if (UseMaterialOverride) { return MO_Metallic; }
    else { return gbuffer.material.metal; }
}

float Material_GetRoughness(GBufferValues gbuffer)
{
    if (UseMaterialOverride) { return MO_Roughness; }
    else { return gbuffer.material.roughness; }
}

float Material_GetSpecular0(GBufferValues gbuffer)
{
    if (UseMaterialOverride) { return MO_Specular; }
    else { return gbuffer.material.specular; }
}

float Material_GetSpecular1(GBufferValues gbuffer)
{
    if (UseMaterialOverride) { return MO_Specular2; }
    else { return 0.f; }
}

float Material_SpecularToF0(float specularParameter)
{
    return RefractiveIndexToF0(lerp(1.0f, 2.5f, specularParameter));
}

float Material_GetF0_0(GBufferValues gbuffer)
{
        //	Note that we don't need to do the conversion from
        //	from refractive index -> F0 here. We could start
        //	working with F0 values directly.
    return Material_SpecularToF0(Material_GetSpecular0(gbuffer));
}

float Material_GetF0_1(GBufferValues gbuffer)
{
    return Material_SpecularToF0(Material_GetSpecular1(gbuffer));
}

float Material_GetDiffuseScale(GBufferValues gbuffer)
{
    if (UseMaterialOverride) { return MO_DiffuseScale; }
    else { return 1.f; }
}

float Material_GetSpecularScale0(GBufferValues gbuffer)
{
    if (UseMaterialOverride) { return MO_Specular0Scale; }
    else { return 1.f; }
}

float Material_GetSpecularScale1(GBufferValues gbuffer)
{
    if (UseMaterialOverride) { return MO_Specular1Scale; }
    else { return 0.f; }
}

float Material_GetReflectionScale(GBufferValues gbuffer)
{
    if (UseMaterialOverride) { return MO_ReflectionsScale; }
    else { return 1.f; }
}

float Material_GetReflectionBoost(GBufferValues gbuffer)
{
    if (UseMaterialOverride) { return MO_ReflectionsBoost; }
    else { return 0.f; }
}

#endif
