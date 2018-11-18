﻿#pragma once
#include "../Graphics/Common.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/DepthBuffer.hpp"
#include "../Shader/ShaderInterfaceFramework.hpp"

namespace ln {
class GraphicsContext;
namespace detail {
class RenderingManager;
class FrameBufferCache;
class DrawElementList;
class DrawElementListCollector;
class SceneRenderer;
}

struct FrameBuffer
{
	Ref<RenderTargetTexture> renderTarget[detail::MaxMultiRenderTargets];
	Ref<DepthBuffer> depthBuffer;
};

// RenderView は別の RenderingContext の描画コマンド構築中に、レンダリングターゲットを生成する目的で render を実行することがある。
// そのため render の実装は RenderingContext や GraphicsContext の状態に依存しないようにしなければならない。
class RenderView
	: public Object
{
public:
	void initialize();

	// World の描画などでは RenderView のインスタンスを持ち回り、描画してほしい人が Manager を ここに add していく。
	void clearDrawElementListManagers();
	void addDrawElementListManager(detail::DrawElementListCollector* elementListManager);

    virtual void render(GraphicsContext* graphicsContext) = 0;

	// TODO: internal
	//detail::CameraInfo mainCameraInfo;

    // フレーム開始時に決定
    const Size& actualPixelSize() const { return m_actualPixelSize; }

protected:
   

LN_INTERNAL_ACCESS:
	const List<detail::DrawElementListCollector*>& elementListManagers() const { return m_elementListManagers; }
    void setActualPixelSize(const Size& size) { m_actualPixelSize = size; }

private:
	detail::RenderingManager* m_manager;

    // TODO: これ List じゃなくていい気がする、というか、List じゃないほうが安全
	List<detail::DrawElementListCollector*> m_elementListManagers;

    Size m_actualPixelSize;
};

} // namespace ln

