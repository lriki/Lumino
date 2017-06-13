﻿
#include "../Internal.h"
#include <Lumino/Graphics/GraphicsException.h>
#include <Lumino/Graphics/VertexDeclaration.h>
#include <Lumino/Graphics/VertexBuffer.h>
#include <Lumino/Graphics/IndexBuffer.h>
#include <Lumino/Mesh/Mesh.h>
#include "../Graphics/Device/GraphicsDriverInterface.h"
#include "../Graphics/RenderingCommand.h"
#include "../Graphics/GraphicsManager.h"
#include "MeshRenderFeature.h"

LN_NAMESPACE_BEGIN
namespace detail {

//------------------------------------------------------------------------------
MeshRenderFeature::MeshRenderFeature()
	: m_manager(nullptr)
	, m_renderer(nullptr)
{
}

//------------------------------------------------------------------------------
MeshRenderFeature::~MeshRenderFeature()
{
}

//------------------------------------------------------------------------------
void MeshRenderFeature::initialize(GraphicsManager* manager)
{
	if (LN_CHECK_ARG(manager != nullptr)) return;
	m_manager = manager;

	Driver::IGraphicsDevice* device = m_manager->getGraphicsDevice();
	m_renderer = device->getRenderer();
}

//------------------------------------------------------------------------------
void MeshRenderFeature::DrawMesh(MeshResource* mesh, int startIndex, int primitiveCount, PrimitiveType primitiveType)
{
	if (LN_CHECK_ARG(mesh != nullptr)) return;
	auto* _this = this;
	
	VertexDeclaration* decls;
	VertexBuffer* vb[Driver::MaxVertexStreams] = {};
	int vbCount;
	IndexBuffer* ib;
	mesh->CommitRenderData(&decls, vb, &vbCount, &ib);

	DrawMeshCommandData data;
	data.vertexDeclaration = decls->getDeviceObject();
	for (int i = 0; i < vbCount; ++i)
	{
		data.vertexBuffers[i] = vb[i]->resolveRHIObject();
	}
	data.vertexBuffersCount = vbCount;
	data.indexBuffer = ib->resolveRHIObject();
	data.startIndex = startIndex;
	data.primitiveCount = primitiveCount;
	data.primitiveType = primitiveType;
	LN_ENQUEUE_RENDER_COMMAND_2(
		flushState, m_manager,
		MeshRenderFeature*, _this,
		DrawMeshCommandData, data,
		{
			_this->DrawMeshImpl(data);
		});
}

//------------------------------------------------------------------------------
void MeshRenderFeature::DrawMeshImpl(const DrawMeshCommandData& data)
{
	m_renderer->setVertexDeclaration(data.vertexDeclaration);
	for (int i = 0; i < data.vertexBuffersCount; ++i)
	{
		m_renderer->setVertexBuffer(i, data.vertexBuffers[i]);
	}
	m_renderer->setIndexBuffer(data.indexBuffer);
	m_renderer->drawPrimitiveIndexed(data.primitiveType, data.startIndex, data.primitiveCount);
}

} // namespace detail
LN_NAMESPACE_END
