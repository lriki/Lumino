﻿
#include "Internal.hpp"
#include <LuminoEngine/Graphics/Texture.hpp>
#include <LuminoEngine/Graphics/SamplerState.hpp>
#include <LuminoEngine/Rendering/Material.hpp>
#include <LuminoEngine/Rendering/RenderingContext.hpp>
#include <LuminoEngine/Rendering/RenderView.hpp>
#include <LuminoEngine/ImageEffect/SSRImageEffect.hpp>
#include "../Rendering/RenderingManager.hpp"

namespace ln {

//==============================================================================
// SSRImageEffect

SSRImageEffect::SSRImageEffect()
{
}

void SSRImageEffect::init()
{
    ImageEffect::init();
}

Ref<ImageEffectInstance> SSRImageEffect::onCreateInstance()
{
    return makeObject<detail::SSRImageEffectInstance>(this);
}

//==============================================================================
// SSRImageEffectInstance

namespace detail {

SSRImageEffectInstance::SSRImageEffectInstance()
    : m_owner(nullptr)
    , m_viewWidth(0)
    , m_viewHeight(0)
{
}

bool SSRImageEffectInstance::init(SSRImageEffect* owner)
{
    if (!ImageEffectInstance::init()) return false;

    m_owner = owner;

    auto shader1 = EngineDomain::renderingManager()->builtinShader(BuiltinShader::SSRRayTracing);
    m_ssrMaterial = makeObject<Material>();
    m_ssrMaterial->setShader(shader1);
    m_ssrMaterial_ColorSampler = shader1->findParameter(u"_ColorSampler");
    m_ssrMaterial_NormalAndDepthSampler = shader1->findParameter(u"_NormalAndDepthSampler");
    m_ssrMaterial_ViewDepthSampler = shader1->findParameter(u"_ViewDepthSampler");
    m_ssrMaterial_MetalRoughSampler = shader1->findParameter(u"_MetalRoughSampler");

    auto shader2 = EngineDomain::renderingManager()->builtinShader(BuiltinShader::SSRBlur);
    m_ssrBlurMaterial1 = makeObject<Material>();
    m_ssrBlurMaterial1->setShader(shader2);

    m_ssrBlurMaterial2 = makeObject<Material>();
    m_ssrBlurMaterial2->setShader(shader2);

    auto shader3 = EngineDomain::renderingManager()->builtinShader(BuiltinShader::SSRComposite);
    m_ssrCompositeMaterial = makeObject<Material>();
    m_ssrCompositeMaterial->setShader(shader3);
    m_paramColorSampler = shader3->findParameter(u"_ColorSampler");
    m_paramSSRSampler = shader3->findParameter(u"_SSRSampler");

    // TODO: 他と共有したいところ
    m_samplerState = makeObject<SamplerState>(TextureFilterMode::Linear, TextureAddressMode::Clamp);

    return true;
}

bool SSRImageEffectInstance::onRender(RenderingContext* context, RenderTargetTexture* source, RenderTargetTexture* destination)
{
    Texture* viewNormalMap = context->gbuffer(GBuffer::ViewNormalMap);
    Texture* viewDepthMap = context->gbuffer(GBuffer::ViewDepthMap);
    Texture* viewMaterialMap = context->gbuffer(GBuffer::ViewMaterialMap);

    if (viewNormalMap && viewDepthMap && viewMaterialMap) {

        int resx = source->width();
        int resy = source->height();
        if (m_viewWidth != resx || m_viewHeight != resy) {
            resetResources(resx, resy);
        }

        m_ssrMaterial_ColorSampler->setTexture(source);
        m_ssrMaterial_NormalAndDepthSampler->setTexture(viewNormalMap);
        m_ssrMaterial_ViewDepthSampler->setTexture(viewDepthMap);
        m_ssrMaterial_MetalRoughSampler->setTexture(viewMaterialMap);
        context->blit(m_ssrMaterial, m_ssrTarget);

#if 0   // DEBUG: Ray-trace result
        context->blit(m_ssrMaterial, destination);
        return true;
#endif
        m_ssrBlurMaterial1->setMainTexture(m_ssrTarget);
        context->blit(m_ssrBlurMaterial1, m_blurTarget1);

        m_ssrBlurMaterial2->setMainTexture(m_blurTarget1);
        context->blit(m_ssrBlurMaterial2, m_blurTarget2);

        m_paramColorSampler->setTexture(source);
        m_paramSSRSampler->setTexture(m_blurTarget2);
        context->blit(m_ssrCompositeMaterial, destination);

        return true;
    }
    else {
        return false;
    }
}

void SSRImageEffectInstance::resetResources(int resx, int resy)
{
    // TODO: tempolary からとっていいかも
    m_ssrTarget = makeObject<RenderTargetTexture>(resx, resy, TextureFormat::RGBA8, false);
    m_ssrTarget->setSamplerState(m_samplerState);
    m_blurTarget1 = makeObject<RenderTargetTexture>(resx, resy, TextureFormat::RGBA8, false);
    m_blurTarget1->setSamplerState(m_samplerState);
    m_blurTarget2 = makeObject<RenderTargetTexture>(resx, resy, TextureFormat::RGBA8, false);
    m_blurTarget2->setSamplerState(m_samplerState);
    m_compositeTarget = makeObject<RenderTargetTexture>(resx, resy, TextureFormat::RGBA8, false);
    m_compositeTarget->setSamplerState(m_samplerState);

    m_viewWidth = resx;
    m_viewHeight = resy;
}

} // namespace detail
} // namespace ln

