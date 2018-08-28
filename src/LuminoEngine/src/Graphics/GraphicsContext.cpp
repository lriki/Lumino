﻿
#include "Internal.hpp"
#include <Lumino/Graphics/GraphicsContext.hpp>
#include <Lumino/Graphics/VertexDeclaration.hpp>
#include <Lumino/Graphics/VertexBuffer.hpp>
#include <Lumino/Graphics/IndexBuffer.hpp>
#include <Lumino/Graphics/Texture.hpp>
#include <Lumino/Graphics/DepthBuffer.hpp>
#include <Lumino/Shader/Shader.hpp>
#include "GraphicsManager.hpp"
#include "GraphicsDeviceContext.hpp"
#include "../Engine/RenderingCommandList.hpp"

namespace ln {

//==============================================================================
// SwapChain

SwapChain::SwapChain()
	: m_rhiObject(nullptr)
	, m_colorBuffer(nullptr)
{
}

SwapChain::~SwapChain()
{
}

void SwapChain::initialize(detail::PlatformWindow* window, const SizeI& backbufferSize)
{
	// TODO: GraphicsResource にして、onChangeDevice でバックバッファをアタッチ
	Object::initialize();
	m_rhiObject = detail::EngineDomain::graphicsManager()->deviceContext()->createSwapChain(window, backbufferSize);
	m_colorBuffer = newObject<RenderTargetTexture>(m_rhiObject->getColorBuffer());
	m_depthBuffer = newObject<DepthBuffer>(m_colorBuffer->width(), m_colorBuffer->height());
}

void SwapChain::dispose()
{
	m_rhiObject.reset();
	m_depthBuffer.reset();
	m_colorBuffer.reset();
	Object::dispose();
}

RenderTargetTexture* SwapChain::colorBuffer() const
{
	return m_colorBuffer;
}

DepthBuffer* SwapChain::depthBuffer() const
{
	return m_depthBuffer;
}

void SwapChain::wait()
{
	// TODO
}

detail::ISwapChain* SwapChain::resolveRHIObject() const
{
	return m_rhiObject;
}

//==============================================================================
// GraphicsContext

GraphicsContext::GraphicsContext()
	: m_device(nullptr)
{
}

GraphicsContext::~GraphicsContext()
{
}

void GraphicsContext::initialize(detail::IGraphicsDeviceContext* device)
{
	m_manager = detail::EngineDomain::graphicsManager();
	m_device = device;

	m_staging.reset();
	m_current.reset();
}

void GraphicsContext::dispose()
{
}

void GraphicsContext::setBlendState(const BlendStateDesc& value)
{
	m_staging.blendState = value;
}

void GraphicsContext::setRasterizerState(const RasterizerStateDesc& value)
{
	m_staging.rasterizerState = value;
}

void GraphicsContext::setDepthStencilState(const DepthStencilStateDesc& value)
{
	m_staging.depthStencilState = value;
}

void GraphicsContext::setColorBuffer(int index, RenderTargetTexture* value)
{
	m_staging.renderTargets[index] = value;
}

void GraphicsContext::setDepthBuffer(DepthBuffer* value)
{
	m_staging.depthBuffer = value;
}

void GraphicsContext::setVertexDeclaration(VertexDeclaration* value)
{
	m_staging.vertexDeclaration = value;
}

void GraphicsContext::setVertexBuffer(int streamIndex, VertexBuffer* value)
{
	m_staging.vertexBuffers[streamIndex] = value;
}

void GraphicsContext::setIndexBuffer(IndexBuffer* value)
{
	m_staging.indexBuffer = value;
}

void GraphicsContext::setShaderPass(ShaderPass* pass)
{
	if (pass)
	{
		m_staging.shader = pass->shader();
		m_staging.shaderPass = pass;
	}
	else
	{
		m_staging.shader = nullptr;
		m_staging.shaderPass = nullptr;
	}
}

void GraphicsContext::clear(ClearFlags flags, const Color& color, float z, uint8_t stencil)
{
	commitStatus();
	// TODO: threading
	m_device->clearBuffers(flags, color, z, stencil);
}

void GraphicsContext::drawPrimitive(PrimitiveType primitive, int startVertex, int primitiveCount)
{
	commitStatus();
	LN_ENQUEUE_RENDER_COMMAND_4(
		GraphicsContext_setIndexBuffer, m_manager,
		detail::IGraphicsDeviceContext*, m_device,
		PrimitiveType, primitive,
		int, startVertex,
		int, primitiveCount,
		{
			m_device->drawPrimitive(primitive, startVertex, primitiveCount);
		});
}

void GraphicsContext::drawPrimitiveIndexed(PrimitiveType primitive, int startIndex, int primitiveCount)
{
	commitStatus();
	LN_ENQUEUE_RENDER_COMMAND_4(
		GraphicsContext_setIndexBuffer, m_manager,
		detail::IGraphicsDeviceContext*, m_device,
		PrimitiveType, primitive,
		int, startIndex,
		int, primitiveCount,
		{
			m_device->drawPrimitiveIndexed(primitive, startIndex, primitiveCount);
		});
}

void GraphicsContext::present(SwapChain* swapChain)
{
	if (LN_REQUIRE(swapChain)) return;

	// TODO: threading
	m_device->present(swapChain->resolveRHIObject());
}

void GraphicsContext::commitStatus()
{
	// ポインタとしては変わっていなくても、resolve は毎回呼び出す。
	// こうしておかないと、
	// 頂点バッファset > 描画 > 頂点バッファ更新 > 描画
	// といったように、同じオブジェクトを set したまま内容を更新した場合に反映されなくなる。

	// TODO: ぜんぶまとめて送信できる方法も用意していい気がする。

	{
		auto& blendState = m_staging.blendState;
		auto& rasterizerState = m_staging.rasterizerState;
		auto& depthStencilState = m_staging.depthStencilState;
		LN_ENQUEUE_RENDER_COMMAND_4(
			GraphicsContext_setPipelineState, m_manager,
			detail::IGraphicsDeviceContext*, m_device,
			BlendStateDesc, blendState,
			RasterizerStateDesc, rasterizerState,
			DepthStencilStateDesc, depthStencilState,
			{
				m_device->setBlendState(blendState);
				m_device->setRasterizerState(rasterizerState);
				m_device->setDepthStencilState(depthStencilState);
			});
	}

	{
		for (int i = 0; i < m_staging.renderTargets.size(); i++)
		{
			auto& value = m_staging.renderTargets[i];
			detail::ITexture* rhiObject = (value) ? value->resolveRHIObject() : nullptr;
			LN_ENQUEUE_RENDER_COMMAND_3(
				GraphicsContext_setDepthBuffer, m_manager,
				detail::IGraphicsDeviceContext*, m_device,
				int, i,
				detail::ITexture*, rhiObject,
				{
					m_device->setColorBuffer(i, rhiObject);
				});
		}
	}

	{
		auto& value = m_staging.depthBuffer;
		detail::IDepthBuffer* rhiObject = (value) ? value->resolveRHIObject() : nullptr;
		LN_ENQUEUE_RENDER_COMMAND_2(
			GraphicsContext_setDepthBuffer, m_manager,
			detail::IGraphicsDeviceContext*, m_device,
			detail::IDepthBuffer*, rhiObject,
			{
				m_device->setDepthBuffer(rhiObject);
			});
	}

	{
		auto& value = m_staging.vertexDeclaration;
		detail::IVertexDeclaration* rhiObject = (value) ? value->resolveRHIObject() : nullptr;
		LN_ENQUEUE_RENDER_COMMAND_2(
			GraphicsContext_setVertexDeclaration, m_manager,
			detail::IGraphicsDeviceContext*, m_device,
			detail::IVertexDeclaration*, rhiObject,
			{
				m_device->setVertexDeclaration(rhiObject);
			});
	}

	{
		for (int i = 0; i < m_staging.vertexBuffers.size(); i++)
		{
			auto& value = m_staging.vertexBuffers[i];
			detail::IVertexBuffer* rhiObject = (value) ? value->resolveRHIObject() : nullptr;
			LN_ENQUEUE_RENDER_COMMAND_3(
				GraphicsContext_setVertexBuffer, m_manager,
				detail::IGraphicsDeviceContext*, m_device,
				int, i,
				detail::IVertexBuffer*, rhiObject,
				{
					m_device->setVertexBuffer(i, rhiObject);
				});
		}
	}

	{
		auto& value = m_staging.indexBuffer;
		detail::IIndexBuffer* rhiObject = (value) ? value->resolveRHIObject() : nullptr;
		LN_ENQUEUE_RENDER_COMMAND_2(
			GraphicsContext_setIndexBuffer, m_manager,
			detail::IGraphicsDeviceContext*, m_device,
			detail::IIndexBuffer*, rhiObject,
			{
				m_device->setIndexBuffer(rhiObject);
			});
	}

	{
		auto& value = m_staging.shaderPass;
		detail::IShaderPass* rhiObject = (value) ? value->resolveRHIObject() : nullptr;
		LN_ENQUEUE_RENDER_COMMAND_2(
			GraphicsContext_setShaderPass, m_manager,
			detail::IGraphicsDeviceContext*, m_device,
			detail::IShaderPass*, rhiObject,
			{
				m_device->setShaderPass(rhiObject);
			});
	}
}

void GraphicsContext::State::reset()
{
	blendState = BlendStateDesc();
	rasterizerState = RasterizerStateDesc();
	depthStencilState = DepthStencilStateDesc();
	renderTargets = {};
	depthBuffer = nullptr;
	vertexDeclaration = nullptr;
	vertexBuffers = {};
	indexBuffer = nullptr;
	shaderPass = nullptr;
}

} // namespace ln

