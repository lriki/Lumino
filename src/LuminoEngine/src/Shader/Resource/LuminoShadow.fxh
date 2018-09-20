
#ifndef LUMINO_SHADOW_INCLUDED
#define LUMINO_SHADOW_INCLUDED

Texture2D         ln_DirectionalShadowMap;
SamplerState    ln_DirectionalShadowMapSamplerState;
float2          ln_DirectionalShadowMapPixelSize;

static float2   ln_BlurPixelStep = float2(1.0, 1.0) / ln_DirectionalShadowMapPixelSize;
static float2   ln_DX9ShadowMapHalfPixelStep = float2(0.5, 0.5) / ln_DirectionalShadowMapPixelSize;

/*
sampler2D ln_DirectionalShadowMap_Sampler = sampler_state
{
    Texture = <ln_DirectionalShadowMap>;
    MinFilter = Point; 
    MagFilter = Point;
    MipFilter = None;
    AddressU = Clamp;
    AddressV = Clamp;
};
*/

// シャドウマップからサンプリングした値が compare より奥にあれば 1(影をつける)、そうでなければ 0
float LN_CompareShadowTexture(float2 uv, float compareZ)
{
	return step(compareZ, (ln_DirectionalShadowMap.Sample(ln_DirectionalShadowMapSamplerState, uv).r));
}

// posInLight : Position in viewport coord of light
float LN_CalculateShadow(float4 posInLight)
{
    float2 shadowUV = 0.5 * (posInLight.xy / posInLight.w) + float2(0.5, 0.5);
    shadowUV.y = 1.0 - shadowUV.y;

#ifdef LN_HLSL_DX9
    shadowUV += ln_DX9ShadowMapHalfPixelStep;
#endif

#if 1
    float2 s2 = ln_BlurPixelStep;
    float depth = posInLight.z/ posInLight.w;
    float compareZ = depth - 0.0065;
    float shadow = (
        LN_CompareShadowTexture(shadowUV + float2(-s2.x, -s2.y), compareZ) +
        LN_CompareShadowTexture(shadowUV + float2(0    , -s2.y), compareZ) +
        LN_CompareShadowTexture(shadowUV + float2( s2.x, -s2.y), compareZ) +
        LN_CompareShadowTexture(shadowUV + float2(-s2.x, 0    ), compareZ) +
        LN_CompareShadowTexture(shadowUV + float2(0    , 0    ), compareZ) +
        LN_CompareShadowTexture(shadowUV + float2(+s2.x, 0    ), compareZ) +
        LN_CompareShadowTexture(shadowUV + float2(-s2.x,  s2.y), compareZ) +
        LN_CompareShadowTexture(shadowUV + float2(0    ,  s2.y), compareZ) +
        LN_CompareShadowTexture(shadowUV + float2( s2.x,  s2.y), compareZ)
    ) * ( 1.0 / 9.0 );

    return lerp(0.5, 1.0, shadow);

#else
    float shadow = ln_DirectionalShadowMap.Sample(ln_DirectionalShadowMapSamplerState, shadowUV).r;

    float depth = posInLight.z/ posInLight.w;

    if (depth > shadow + 0.0065)
    {
        outgoingLight *= 0.5;
    }
#endif
}

#endif // LUMINO_SHADOW_INCLUDED
