
#pragma once
#include "ImageEffect.h"

LN_NAMESPACE_BEGIN
LN_NAMESPACE_GRAPHICS_BEGIN

/**
	@brief
*/
class ScreenMotionBlurImageEffect
	: public ImageEffect
{
public:
	static ScreenMotionBlurImageEffectPtr Create();

protected:
	ScreenMotionBlurImageEffect();
	virtual ~ScreenMotionBlurImageEffect();
	void Initialize(GraphicsManager* manager);
	virtual void OnRender(RenderingContext2* renderingContext, RenderTarget* source, RenderTarget* destination) override;

	GraphicsManager*	m_manager;
	Shader*				m_shader;
	RenderTarget*		m_accumTexture;	// 前回の画面描画内容
};

LN_NAMESPACE_GRAPHICS_END
LN_NAMESPACE_END
