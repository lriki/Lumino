
#include "../Internal.h"
#include <Lumino/Graphics/Brush.h>
#include <Lumino/Graphics/Rendering.h>
#include <Lumino/Graphics/VertexDeclaration.h>
#include "../Device/GraphicsDriverInterface.h"
#include "../GraphicsManager.h"
#include "../RenderingCommand.h"
#include "ShapesRenderer.h"

LN_NAMESPACE_BEGIN
namespace detail {

//==============================================================================
// ShapesRendererCommandList
//==============================================================================
//------------------------------------------------------------------------------
void ShapesRendererCommandList::AddDrawBoxBorder(
	float x, float y, float w, float h, float l, float t, float r, float b,
	const Color& leftColor, const Color& topColor, const Color& rightColor, const Color& bottomColor,
	float ltRad, float rtRad, float lbRad, float rbRad)
{
	float cmd[] =
	{
		(float)Cmd_DrawBoxBorder, x, y, w, h, l, t, r, b,
		// [9]
		leftColor.r, leftColor.g, leftColor.b, leftColor.a,
		topColor.r, topColor.g, topColor.b, topColor.a,
		rightColor.r, rightColor.g, rightColor.b, rightColor.a,
		bottomColor.r, bottomColor.g, bottomColor.b, bottomColor.a,
		// [25]
		ltRad, rtRad, lbRad, rbRad
	};
	AllocData(sizeof(cmd), cmd);
}

//------------------------------------------------------------------------------
void ShapesRendererCommandList::AddDrawBoxShadow(float x, float y, float w, float h, const Color& color, float blur, float width, bool inset)
{
	float cmd[] =
	{
		(float)Cmd_DrawBoxShadow, x, y, w, h,
		color.r, color.g, color.b, color.a,
		blur, width, (inset) ? 1.0f : 0.0f
	};
	AllocData(sizeof(cmd), cmd);
}

//==============================================================================
// ShapesRendererCommandListCache
//==============================================================================

//------------------------------------------------------------------------------
RefPtr<ShapesRendererCommandList> ShapesRendererCommandListCache::CreateObject()
{
	return RefPtr<ShapesRendererCommandList>::MakeRef();
}

//==============================================================================
// ShapesRendererCore
//==============================================================================
//------------------------------------------------------------------------------
ShapesRendererCore::ShapesRendererCore()
	: m_manager(nullptr)
	, m_vertexBuffer(nullptr)
	, m_indexBuffer(nullptr)
	, m_points()
	, m_vertexCache()
	, m_indexCache()
{
}

//------------------------------------------------------------------------------
ShapesRendererCore::~ShapesRendererCore()
{
	LN_SAFE_RELEASE(m_vertexBuffer);
	LN_SAFE_RELEASE(m_indexBuffer);
}

//------------------------------------------------------------------------------
void ShapesRendererCore::Initialize(GraphicsManager* manager)
{
	m_manager = manager;
	m_points.Reserve(4096);
	m_vertexCache.Reserve(4096);
	m_indexCache.Reserve(4096);
}

//------------------------------------------------------------------------------
void ShapesRendererCore::RequestBuffers(int vertexCount, int indexCount, Vertex** vb, uint16_t** ib, uint16_t* outBeginVertexIndex)
{
	////assert(vb != nullptr);
	////assert(ib != nullptr);
	////*outBeginVertexIndex = m_vertexCache.GetCount();
	////*vb = m_vertexCache.Request(vertexCount);
	////*ib = m_indexCache.Request(indexCount);
}

//------------------------------------------------------------------------------
void ShapesRendererCore::RenderCommandList(ShapesRendererCommandList* commandList, detail::BrushRawData* fillBrush)
{
	ExtractBasePoints(commandList);
	CalcExtrudedDirection();
	ExpandFill();

	//for (int i = 0; i < dataCount; i++)
	//{
	//	Vertex* vb;
	//	uint16_t* ib;
	//	uint16_t beginVertexIndex;
	//	RequestBuffers(
	//		cache->GetVertexCount(dataList[i].cacheGlyphInfoHandle),
	//		cache->GetIndexCount(dataList[i].cacheGlyphInfoHandle),
	//		&vb, &ib, &beginVertexIndex);
	//	cache->GenerateMesh(
	//		dataList[i].cacheGlyphInfoHandle, Vector3(dataList[i].origin.x, dataList[i].origin.y, 0), dataList[i].transform,
	//		vb, ib, beginVertexIndex);
	//}

	// TODO: このへん PrimitiveRenderer と同じ。共通にできないか？
	{
		Driver::IRenderer* renderer = m_manager->GetGraphicsDevice()->GetRenderer();

		// サイズが足りなければ再作成
		auto* device = m_manager->GetGraphicsDevice();
		if (m_vertexBuffer == nullptr || m_vertexBuffer->GetByteCount() < m_vertexCache.GetBufferUsedByteCount())
		{
			LN_SAFE_RELEASE(m_vertexBuffer);
			m_vertexBuffer = device->CreateVertexBuffer(m_vertexCache.GetBufferUsedByteCount(), nullptr, ResourceUsage::Dynamic);
		}
		if (m_indexBuffer == nullptr || m_indexBuffer->GetByteCount() < m_indexCache.GetBufferUsedByteCount())
		{
			LN_SAFE_RELEASE(m_indexBuffer);
			m_indexBuffer = device->CreateIndexBuffer(m_indexCache.GetBufferUsedByteCount(), nullptr, IndexBufferFormat_UInt16, ResourceUsage::Dynamic);
		}

		// 描画する
		m_vertexBuffer->SetSubData(0, m_vertexCache.GetBuffer(), m_vertexCache.GetBufferUsedByteCount());
		m_indexBuffer->SetSubData(0, m_indexCache.GetBuffer(), m_indexCache.GetBufferUsedByteCount());

		{
			renderer->SetVertexDeclaration(m_manager->GetDefaultVertexDeclaration()->GetDeviceObject());
			renderer->SetVertexBuffer(0, m_vertexBuffer);
			renderer->SetIndexBuffer(m_indexBuffer);
			renderer->DrawPrimitiveIndexed(PrimitiveType_TriangleList, 0, m_indexCache.GetCount() / 3);
		}
	}

	// キャッシュクリア
	m_vertexCache.Clear();
	m_indexCache.Clear();
	m_points.Clear();
	m_pathes.Clear();
}

//------------------------------------------------------------------------------
void ShapesRendererCore::ReleaseCommandList(ShapesRendererCommandList* commandList)
{
	commandList->Clear();
	m_manager->GetShapesRendererCommandListCache()->ReleaseCommandList(commandList);
}

//------------------------------------------------------------------------------
ShapesRendererCore::Path* ShapesRendererCore::AddPath()
{
	m_pathes.Add(Path{ m_points.GetCount(), 0, Color(0, 0, 0, 1) });
	return &m_pathes.GetLast();
}

//------------------------------------------------------------------------------
void ShapesRendererCore::EndPath(Path* path, bool close)
{
	path->pointCount = m_points.GetCount() - path->pointStart;
	path->close = close;
}

//------------------------------------------------------------------------------
void ShapesRendererCore::ExtractBasePoints(ShapesRendererCommandList* commandList)
{
	int count = commandList->GetDataCount();
	for (int i = 0; i < count; i++)
	{
		float* cmd = (float*)commandList->GetDataByIndex(i);
		switch ((int)cmd[0])
		{
			case ShapesRendererCommandList::Cmd_DrawBoxBorder:
			{
				float il = cmd[1];
				float it = cmd[2];
				float ir = cmd[1] + cmd[3];
				float ib = cmd[2] + cmd[4];
				float ol = il - cmd[5];
				float ot = it - cmd[6];
				float or = ir + cmd[7];
				float ob = ib + cmd[8];
				float ltRad = cmd[25];
				float rtRad = cmd[26];
				float lbRad = cmd[27];
				float rbRad = cmd[28];
				const float rtir = 0.55228f;	// https://cat-in-136.github.io/2014/03/bezier-1-kappa.html
				if (0){	// left
					auto* path = AddPath();
					path->color = Color(cmd[9], cmd[10], cmd[11], cmd[12]);

					// left-curve
					m_points.Add({ Vector3(ol, ot, 0) });	// left-top

					

					m_points.Add({ Vector3(ol, ob, 0) });	// left-bottom
					m_points.Add({ Vector3(il, ib, 0) });	// right-bottom
					m_points.Add({ Vector3(il, it, 0) });	// right-top
					EndPath(path, true);
				}
				{	// top
					auto* path = AddPath();
					path->color = Color(cmd[13], cmd[14], cmd[15], cmd[16]);
					m_points.Add({ Vector3(ol, ot, 0) });	// left-top

					Vector2 cp2(il - ltRad * rtir, ot), cp3(ol, it - ltRad * rtir);
					int tess = 4;
					float incr = 0.5 / tess;
					for (float t = 0.0f; t < 0.5f; t += incr)
					{
						m_points.Add({ Vector3(
							Math::CubicBezier(il, cp2.x, cp3.x, ol, t),
							Math::CubicBezier(ot, cp2.y, cp3.y, it, t),
							0) });
					}
					m_points.Add({ Vector3(
						Math::CubicBezier(il, cp2.x, cp3.x, ol, 0.5f),
						Math::CubicBezier(ot, cp2.y, cp3.y, it, 0.5f),
						0) });



					m_points.Add({ Vector3(il, it, 0) });	// left-bottom
					m_points.Add({ Vector3(ir, it, 0) });	// right-bottom
					m_points.Add({ Vector3(or, ot, 0) });	// right-top
					EndPath(path, true);
				}
				//{	// right
				//	auto* path = AddPath();
				//	path->color = Color(cmd[17], cmd[18], cmd[19], cmd[20]);
				//	m_points.Add({ Vector3(ir, it, 0) });	// left-top
				//	m_points.Add({ Vector3(ir, ib, 0) });	// left-bottom
				//	m_points.Add({ Vector3(or, ob, 0) });	// right-bottom
				//	m_points.Add({ Vector3(or, ot, 0) });	// right-top
				//	EndPath(path, true);
				//}
				//{	// bottom
				//	auto* path = AddPath();
				//	path->color = Color(cmd[21], cmd[22], cmd[23], cmd[24]);
				//	m_points.Add({ Vector3(il, ib, 0) });	// left-top
				//	m_points.Add({ Vector3(ol, ob, 0) });	// left-bottom
				//	m_points.Add({ Vector3(or, ob, 0) });	// right-bottom
				//	m_points.Add({ Vector3(ir, ib, 0) });	// right-top
				//	EndPath(path, true);
				//}
				break;
			}
			default:
				LN_UNREACHABLE();
				break;
		}
	}
}

//------------------------------------------------------------------------------
void ShapesRendererCore::CalcExtrudedDirection()
{

}

//------------------------------------------------------------------------------
void ShapesRendererCore::ExpandFill()
{
	for (int iPath = 0; iPath < m_pathes.GetCount(); iPath++)
	{
		const Path& path = m_pathes.GetAt(iPath);
		int pointCount = path.pointCount;

		// make VertexBuffer
		for (int i = 0; i < pointCount; i++)
		{
			const BasePoint& pt = m_points.GetAt(path.pointStart + i);
			Vertex v;
			v.position = pt.pos;
			v.color = path.color;
			//v.uv.x = (pt.point.x - posMin.x) * uvSpan.x;
			//v.uv.y = (pt.point.y - posMin.y) * uvSpan.y;
			m_vertexCache.Add(v);
		}

		// make IndexBuffer (反時計回り)
		int ib = path.pointStart;
		int i0 = 0;
		int i1 = 1;
		int i2 = pointCount - 1;
		for (int iPt = 0; iPt < pointCount - 2; iPt++)
		{
			m_indexCache.Add(ib + i0);
			m_indexCache.Add(ib + i1);
			m_indexCache.Add(ib + i2);

			if (iPt & 1) {	// 奇数回
				i0 = i2;
				--i2;
			}
			else {	// 偶数回
				i0 = i1;
				++i1;
			}
			/*
				↑の概要：
				頂点は反時計回りに並んでいることを前提とし、
				前後それぞれの方向からカーソルを進めるようにして三角形を作っていく。

				- 0回目、0,1,5 を結ぶ
				0-5 4
				|/
				1 2 3

				- 1回目、1,2,5 を結ぶ
				0-5 4
				|/|
				1-2 3

				- 3回目、5,2,4 を結ぶ
				0-5-4
				|/|/
				1-2 3

				- 4回目、2,3,4 を結ぶ
				0-5-4
				|/|/|
				1-2-3
			*/
		}
	}
}

//==============================================================================
// ShapesRenderer
//==============================================================================
//------------------------------------------------------------------------------
ShapesRenderer::ShapesRenderer()
	: m_manager(nullptr)
	, m_core(nullptr)
	, m_fillBrush()
{
}

//------------------------------------------------------------------------------
ShapesRenderer::~ShapesRenderer()
{
}

//------------------------------------------------------------------------------
void ShapesRenderer::Initialize(GraphicsManager* manager)
{
	m_manager = manager;
	m_core = RefPtr<ShapesRendererCore>::MakeRef();
	m_core->Initialize(m_manager);
}

//------------------------------------------------------------------------------
void ShapesRenderer::ExecuteCommand(ShapesRendererCommandList* commandList)
{
	if (LN_CHECK_ARG(commandList != nullptr)) return;

	LN_ENQUEUE_RENDER_COMMAND_3(
		ExecuteCommand, m_manager,
		RefPtr<ShapesRendererCore>, m_core,
		RefPtr<ShapesRendererCommandList>, commandList,
		detail::BrushRawData, m_fillBrush,
		{
			m_core->RenderCommandList(commandList, &m_fillBrush);
		});
}
//------------------------------------------------------------------------------
void ShapesRenderer::OnSetState(const DrawElementBatch* state)
{
	if (state->state.GetBrush() != nullptr)
	{
		state->state.GetBrush()->GetRawData(&m_fillBrush);
	}
}

} // namespace detail
LN_NAMESPACE_END
