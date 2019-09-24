﻿
#include "Internal.hpp"
#include <LuminoEngine/Graphics/VertexLayout.hpp>
#include <LuminoEngine/Graphics/VertexBuffer.hpp>
#include <LuminoEngine/Graphics/GraphicsContext.hpp>
#include <LuminoEngine/Rendering/Vertex.hpp>
#include "../Graphics/GraphicsManager.hpp"
#include "BlitRenderFeature.hpp"
#include "RenderingManager.hpp"

namespace ln {
namespace detail {

//==============================================================================
// BlitRenderFeature

BlitRenderFeature::BlitRenderFeature()
	: m_manager(nullptr)
{
}

void BlitRenderFeature::init(RenderingManager* manager)
{
	if (LN_REQUIRE(manager != nullptr)) return;
	m_manager = manager;

    Vertex vertices[4] =
    {
        { Vector3(-1,  1, 0), Vector3::UnitZ, Vector2(0, 0), Color::White },
        { Vector3(1,  1, 0), Vector3::UnitZ, Vector2(1, 0), Color::White },
        { Vector3(-1, -1, 0), Vector3::UnitZ, Vector2(0, 1), Color::White },
        { Vector3(1, -1, 0), Vector3::UnitZ, Vector2(1, 1), Color::White },
    };
    m_vertexBuffer = m_manager->graphicsManager()->deviceContext()->createVertexBuffer(GraphicsResourceUsage::Static, sizeof(vertices), vertices);
    m_vertexDeclaration = m_manager->standardVertexDeclarationRHI();
}

void BlitRenderFeature::blit(GraphicsContext* context)
{
    auto* _this = this;
	ICommandList* c = GraphicsContextInternal::commitState(context);
    LN_ENQUEUE_RENDER_COMMAND_2(
        BlitRenderFeature_blit, context,
        BlitRenderFeature*, _this,
		ICommandList*, c,
        {
            _this->blitImplOnRenderThread(c);
        });
}

RenderFeatureBatch* BlitRenderFeature::submitBatch(GraphicsContext* context)
{
	// TODO:
	return nullptr;
}

void BlitRenderFeature::renderBatch(GraphicsContext* context, RenderFeatureBatch* batch)
{
	LN_NOTIMPLEMENTED();
}

void BlitRenderFeature::blitImplOnRenderThread(ICommandList* context)
{
	context->setVertexDeclaration(m_vertexDeclaration);
	context->setVertexBuffer(0, m_vertexBuffer);
	context->setPrimitiveTopology(PrimitiveTopology::TriangleStrip);
	context->drawPrimitive(0, 2);
}

} // namespace detail
} // namespace ln

