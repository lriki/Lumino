﻿
#include "Internal.hpp"
#include <LuminoEngine/Graphics/GraphicsContext.hpp>
#include <LuminoEngine/Graphics/RenderPass.hpp>
#include <LuminoEngine/Rendering/RenderFeature.hpp>
#include <LuminoEngine/Rendering/Material.hpp>
#include "RenderingManager.hpp"
#include "RenderElement.hpp"
#include "RenderStage.hpp"

namespace ln {
namespace detail {

const BuiltinEffectData BuiltinEffectData::DefaultValue;

//==============================================================================
// IDrawElementListFrameData

IDrawElementListFrameData::IDrawElementListFrameData()
	: m_nextFrameData(nullptr)
{
}

IDrawElementListFrameData::~IDrawElementListFrameData()
{
}

//==============================================================================
// RenderStage

RenderStage::RenderStage()
	: frameBufferStageParameters(nullptr)
	, geometryStageParameters(nullptr)
	, renderFeature(nullptr)
{
}

bool RenderStage::equals(const RenderStage* other) const
{
	return
		frameBufferStageParameters->equals(other->frameBufferStageParameters) &&
		geometryStageParameters->equals(other->geometryStageParameters) &&
		renderFeature == other->renderFeature;
}

void RenderStage::flush(GraphicsContext* context)
{
	if (renderFeature) {
		// TODO: batch list
		renderFeature->submitBatch(context, nullptr);
	}
}

Material* RenderStage::getMaterialFinal(Material* priorityValue, Material* sceneDefaultMaterial) const
{
	// specified drawXXXX()
	if (priorityValue) {

		// 優先マテリアルを使う場合、ステートは基本のマテリアルと同一でなければならない。
		// この時点で、DrawElement は 不透明・半透明  によって描画パスの振り分けが終わっているので、
		// 振り分け結果とつじつまが合わなくなるようなステートにしてはならない。
		// なお、シェーダ変数は異なっていてもかまわない。
		LN_CHECK(geometryStageParameters->m_material->equalStatus(priorityValue));

		return priorityValue;
	}

	// specified context->setMaterial() or meshObj->setMaterial()
	if (geometryStageParameters->m_material) return geometryStageParameters->m_material;

    // default は SceneRenderer に決めてもらう
	return sceneDefaultMaterial;
}

ShadingModel RenderStage::getShadingModelFinal(const Material* finalMaterial) const
{
	assert(finalMaterial);

	// specified meshObj->setXXXX()
	if (geometryStageParameters && geometryStageParameters->shadingModel.hasValue())
		return geometryStageParameters->shadingModel.value();

	//  specified meshObj->getMaterial(0)->setXXXX() etc...
	return finalMaterial->shadingModel;
}

BlendMode RenderStage::getBlendModeFinal(const Material* finalMaterial) const
{
	if (finalMaterial && finalMaterial->getBlendMode().hasValue())
		return finalMaterial->getBlendMode().value();

	// specified context->setXXXX() or meshObj->setXXXX()
	if (geometryStageParameters->m_blendMode.hasValue())
		return geometryStageParameters->m_blendMode.value();

	// default
	return BlendMode::Normal;
}

CullMode RenderStage::getCullingModeFinal(const Material* finalMaterial) const
{
	if (finalMaterial && finalMaterial->getCullingMode().hasValue())
		return finalMaterial->getCullingMode().value();

	// specified context->setXXXX() or meshObj->setXXXX()
	if (geometryStageParameters->m_cullingMode.hasValue())
		return geometryStageParameters->m_cullingMode.value();

	// default
	return CullMode::Back;
}

ComparisonFunc RenderStage::getDepthTestFuncFinal(const Material* finalMaterial) const
{
	if (finalMaterial && finalMaterial->isDepthTestEnabled().hasValue())
		return (finalMaterial->isDepthTestEnabled().value()) ? ComparisonFunc::LessEqual : ComparisonFunc::Always;

	// specified context->setXXXX() or meshObj->setXXXX()
	if (geometryStageParameters->m_depthTestEnabled.hasValue())
		return (geometryStageParameters->m_depthTestEnabled.value()) ? ComparisonFunc::LessEqual : ComparisonFunc::Always;

	// default
    return ComparisonFunc::LessEqual;
}

bool RenderStage::isDepthWriteEnabledFinal(const Material* finalMaterial) const
{
	if (finalMaterial && finalMaterial->isDepthWriteEnabled().hasValue())
		return finalMaterial->isDepthWriteEnabled().value();

	// specified context->setXXXX() or meshObj->setXXXX()
	if (geometryStageParameters->m_depthWriteEnabled.hasValue())
		return geometryStageParameters->m_depthWriteEnabled.value();

	// default
	return true;
}

float RenderStage::getOpacityFinal(RenderDrawElement* element) const
{
    return element->builtinEffectData->opacity;
}

const Color& RenderStage::getColorScaleFinal(RenderDrawElement* element) const
{
    return element->builtinEffectData->colorScale;
}

const Color& RenderStage::getBlendColorFinal(RenderDrawElement* element) const
{
    return element->builtinEffectData->blendColor;
}

const ColorTone& RenderStage::getToneFinal(RenderDrawElement* element) const
{
    return element->builtinEffectData->tone;
}

//void RenderStage::applyFrameBufferStatus(RenderPass* renderPass, const RenderStage* stage, const FrameBuffer& defaultFrameBufferInPass)
//{
//	RenderTargetTexture* renderTarget0 = nullptr;
//
//	// RenderTarget
//	{
//		for (int i = 0; i < MaxMultiRenderTargets; i++)
//		{
//			RenderTargetTexture* target = stage->getRenderTargetFinal(i);
//			if (!target) {
//				target = defaultFrameBufferInPass.renderTarget[i];
//			}
//
//			renderPass->setRenderTarget(i, target);
//
//			if (i == 0) {
//				renderTarget0 = target;
//			}
//		}
//	}
//
//	// DepthBuffer
//	{
//		DepthBuffer* depthBuffer = stage->getDepthBufferFinal();
//		if (depthBuffer) {
//			renderPass->setDepthBuffer(depthBuffer);
//		}
//		else {
//			renderPass->setDepthBuffer(defaultFrameBufferInPass.depthBuffer);
//		}
//	}
//
//	// Viewport
//	{
//		//const RectI& rect = stage->getViewportRectFinal();//getViewportRect();
//		//if (rect.width < 0)
//		//{
//		//	stateManager->setViewport(RectI(0, 0, renderTarget0->getSize()));
//		//}
//		//else
//		//{
//		//	stateManager->setViewport(rect);
//		//}
//		// TODO:Viewport
//		// TODO: m_scissorRect
//	}
//}

void RenderStage::applyGeometryStatus(GraphicsContext* context, const RenderStage* stage, const Material* priorityMaterial)
{
	// BlendState
	{
		BlendMode mode = stage->getBlendModeFinal(priorityMaterial);
		BlendStateDesc state;
		state.independentBlendEnable = false;
		makeBlendMode(mode, &state.renderTargets[0]);
		context->setBlendState(state);
	}
	// RasterizerState
	{
		RasterizerStateDesc state;
		state.fillMode = FillMode::Solid;
		state.cullMode = stage->getCullingModeFinal(priorityMaterial);
		context->setRasterizerState(state);
	}
	// DepthStencilState
	{
		DepthStencilStateDesc state;
		state.depthTestFunc = stage->getDepthTestFuncFinal(priorityMaterial);
		state.depthWriteEnabled = stage->isDepthWriteEnabledFinal(priorityMaterial);
		context->setDepthStencilState(state);
	}
	// Topology
	{
		context->setPrimitiveTopology(stage->geometryStageParameters->primitiveTopology);
	}
}

void RenderStage::makeBlendMode(BlendMode mode, RenderTargetBlendDesc* state)
{
	// もっといろいろ http://d.hatena.ne.jp/Ko-Ta/20070618/p1
	// TODO: アルファも一緒のブレンド方式にしているので、個別指定で改善できそう
	switch (mode)
	{
	case BlendMode::Normal:
		state->blendEnable = false;
		state->sourceBlend = BlendFactor::One;
		state->destinationBlend = BlendFactor::Zero;
		state->blendOp = BlendOp::Add;
		state->sourceBlendAlpha = BlendFactor::One;
		state->destinationBlendAlpha = BlendFactor::Zero;
		state->blendOpAlpha = BlendOp::Add;
		break;
	case BlendMode::Alpha:
		state->blendEnable = true;
		state->sourceBlend = BlendFactor::SourceAlpha;
		state->destinationBlend = BlendFactor::InverseSourceAlpha;
		state->blendOp = BlendOp::Add;
		state->sourceBlendAlpha = BlendFactor::SourceAlpha;
		//state->destinationBlendAlpha = BlendFactor::InverseSourceAlpha;
        state->destinationBlendAlpha = BlendFactor::DestinationAlpha;
		state->blendOpAlpha = BlendOp::Add;
		break;
	case BlendMode::Add:
		state->blendEnable = true;
		state->sourceBlend = BlendFactor::SourceAlpha;
		state->destinationBlend = BlendFactor::One;
		state->blendOp = BlendOp::Add;
		state->sourceBlendAlpha = BlendFactor::SourceAlpha;
		state->destinationBlendAlpha = BlendFactor::One;
		state->blendOpAlpha = BlendOp::Add;
		break;
	case BlendMode::Subtract:
		state->blendEnable = true;
		state->sourceBlend = BlendFactor::SourceAlpha;
		state->destinationBlend = BlendFactor::One;
		state->blendOp = BlendOp::ReverseSubtract;
		state->sourceBlendAlpha = BlendFactor::SourceAlpha;
		state->destinationBlendAlpha = BlendFactor::One;
		state->blendOpAlpha = BlendOp::Add;
		break;
	case BlendMode::Multiply:
		state->blendEnable = true;
		state->sourceBlend = BlendFactor::Zero;	// AlphaDisable (Alpha を別指定できない今の仕様では Alpha を考慮できない)
		state->destinationBlend = BlendFactor::SourceColor;
		state->blendOp = BlendOp::Add;
		state->sourceBlendAlpha = BlendFactor::SourceAlpha;
		state->destinationBlendAlpha = BlendFactor::One;
		state->blendOpAlpha = BlendOp::Add;
		break;
		//case BlendMode_Screen:
		//	m_dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		//	m_dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		//	m_dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHASAT);
		//	m_dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
		//	m_dxDevice->SetRenderState(D3DRS_ALPHAREF, 255);
		//	break;
		//case BlendMode_Reverse:
		//	m_dxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		//	m_dxDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		//	m_dxDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		//	m_dxDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVSRCCOLOR);
		//	m_dxDevice->SetRenderState(D3DRS_ALPHAREF, 1);
		//	break;
	default:
		assert(0);
		break;
	}
}

} // namespace detail
} // namespace ln

