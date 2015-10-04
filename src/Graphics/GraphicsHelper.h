﻿
#pragma once
//#include <Lumino/Graphics/GraphicsManager.h>
//#include <Lumino/Graphics/Renderer.h>
#include <Lumino/Graphics/VertexBuffer.h>
#include <Lumino/Graphics/IndexBuffer.h>
#include <Lumino/Graphics/Texture.h>
#include <Lumino/Graphics/GlyphRun.h>

namespace Lumino
{
LN_NAMESPACE_GRAPHICS_BEGIN

// internal クラス。公開禁止。
class Helper	// TODO: GraphicsHelper
{
public:
	inline static Device::ITexture*			GetDeviceObject(Texture* texture) { return texture->m_deviceObj; }
	inline static Device::IVertexBuffer*	GetDeviceObject(VertexBuffer* vb) { return vb->m_deviceObj; }
	inline static Device::IIndexBuffer*		GetDeviceObject(IndexBuffer* ib) { return ib->m_deviceObj; }

	inline static void						AttachGlyphTextureCache(GlyphRun* gr, Internal::FontGlyphTextureCache* cache) { gr->AttachGlyphTextureCache(cache); }
	inline static Internal::FontGlyphTextureCache* GetGlyphTextureCache(GlyphRun* gr) { return gr->m_glyphTextureCache; }
	inline static TextLayoutResult*			GetGlyphData(GlyphRun* gr) { return gr->m_glyphData; }
};

LN_NAMESPACE_GRAPHICS_END
} // namespace Lumino
