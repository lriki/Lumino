
#pragma once
#include "Texture.h"
#include "ImageEffect/ImageEffect.h"

LN_NAMESPACE_BEGIN

class RenderingCompositionContext
{
public:
	RenderTarget* RequestRenderTarget(int width, int height);
};

//// Camera とかが継承する
//class IRenderingCompositionItem
//{
//public:
//	//void Render(RenderingCompositionContext* context);
//};

class ViewportLayer
	: public Object
{
public:
	ImageEffectList* GetImageEffects() const { return m_imageEffects; }

protected:
	ViewportLayer();
	virtual ~ViewportLayer();

	///// 前描画
	//virtual void PreRender(const SizeF& viewSize) {}

	/// 本描画
	virtual void Render(RenderTarget* renderTarget) = 0;

	/// 後描画
	void PostRender(RenderingContext2* renderingContext, RenderTarget** primaryLayerTarget, RenderTarget** secondaryLayerTarget);

private:
	RefPtr<ImageEffectList>	m_imageEffects;

	friend class Viewport;
};

/**
	@brief		
*/
class Viewport
	: public Object
{
public:

	static Viewport* GetMainWindowViewport();

	void SetBackgroundColor(const Color& color);

	void AddViewportLayer(ViewportLayer* layer) { m_viewportLayerList.Add(layer); }
	void RemoveViewportLayer(ViewportLayer* layer) { m_viewportLayerList.Remove(RefPtr<ViewportLayer>(layer)); }

LN_INTERNAL_ACCESS:	// TODO: いまはとりあえず内部用途
	Viewport();
	virtual ~Viewport();
	void Initialize(GraphicsManager* manager, RenderTarget* renderTarget);
	void Render();

private:
	void TryRemakeLayerTargets();

	GraphicsManager*				m_manager;
	RenderTarget*					m_renderTarget;
	Array<RefPtr<ViewportLayer>>	m_viewportLayerList;
	ColorF							m_backgroundColor;
	RenderTarget*					m_primaryLayerTarget;
	RenderTarget*					m_secondaryLayerTarget;
};

LN_NAMESPACE_END
