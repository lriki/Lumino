﻿
#include "Internal.hpp"
#include <Lumino/Graphics/Texture.hpp>
#include "../Graphics/GraphicsManager.hpp"
#include "SpriteRenderFeature.hpp"
#include "RenderingManager.hpp"

namespace ln {
namespace detail {

//==============================================================================
// InternalSpriteRenderer

InternalSpriteRenderer::InternalSpriteRenderer()
{
}

void InternalSpriteRenderer::initialize(RenderingManager* manager)
{
	m_device = manager->graphicsManager()->deviceContext();
	m_buffersReservedSpriteCount = 0;
	m_vertexDeclaration = manager->standardVertexDeclaration()->resolveRHIObject();
	
	// reserve buffers.
	m_spriteDataList.reserve(2048);
	prepareBuffers(2048);
}

void InternalSpriteRenderer::setState(const State& state)
{
	m_state = state;
	m_viewDirection.set(m_state.viewMatrix.m[0][2], m_state.viewMatrix.m[1][2], m_state.viewMatrix.m[2][2]);
	m_viewInverseMatrix = Matrix::makeInverse(m_state.viewMatrix);
	m_viewPosition = m_viewInverseMatrix.position();
}

void InternalSpriteRenderer::drawRequest(
	const Matrix& transform,
	const Vector2& size,
	const Vector2& anchorRatio,
	const Rect& srcRect,
	const Color& color,
	SpriteBaseDirection baseDir,
	BillboardType billboardType)
{
	SpriteData sprite;

	Vector2 center(size.x * anchorRatio.x, size.y * anchorRatio.y);

	// 3D の場合の頂点座標
	if (baseDir != SpriteBaseDirection::Basic2D)
	{
		//Vector3 origin(-center);
		Vector2 harf_size(size * 0.5f);
		float l, t, r, b;
		r = harf_size.x;
		b = -harf_size.y;
		l = -r;
		t = -b;

		l -= center.x;
		r -= center.x;
		t -= center.y;
		b -= center.y;

#define LN_WRITE_V3( x_, y_, z_ ) x_, y_, z_

		switch (baseDir)
		{
		case SpriteBaseDirection::XPlus:
			sprite.vertices[0].position.set(LN_WRITE_V3(0, t, l));     // 左上
			sprite.vertices[1].position.set(LN_WRITE_V3(0, t, r));     // 右上
			sprite.vertices[2].position.set(LN_WRITE_V3(0, b, l));     // 左下
			sprite.vertices[3].position.set(LN_WRITE_V3(0, b, r));     // 右下
			break;
		case SpriteBaseDirection::YPlus:
			sprite.vertices[0].position.set(LN_WRITE_V3(l, 0, t));
			sprite.vertices[1].position.set(LN_WRITE_V3(r, 0, t));
			sprite.vertices[2].position.set(LN_WRITE_V3(l, 0, b));
			sprite.vertices[3].position.set(LN_WRITE_V3(r, 0, b));
			break;
		case SpriteBaseDirection::ZPlus:
			sprite.vertices[0].position.set(LN_WRITE_V3(r, t, 0));
			sprite.vertices[1].position.set(LN_WRITE_V3(l, t, 0));
			sprite.vertices[2].position.set(LN_WRITE_V3(r, b, 0));
			sprite.vertices[3].position.set(LN_WRITE_V3(l, b, 0));
			break;
		case SpriteBaseDirection::XMinus:
			sprite.vertices[0].position.set(LN_WRITE_V3(0, t, r));
			sprite.vertices[1].position.set(LN_WRITE_V3(0, t, l));
			sprite.vertices[2].position.set(LN_WRITE_V3(0, b, r));
			sprite.vertices[3].position.set(LN_WRITE_V3(0, b, l));
			break;
		case SpriteBaseDirection::YMinus:
			sprite.vertices[0].position.set(LN_WRITE_V3(r, 0, t));
			sprite.vertices[1].position.set(LN_WRITE_V3(l, 0, t));
			sprite.vertices[2].position.set(LN_WRITE_V3(r, 0, b));
			sprite.vertices[3].position.set(LN_WRITE_V3(l, 0, b));
			break;
		case SpriteBaseDirection::ZMinus:
			sprite.vertices[0].position.set(LN_WRITE_V3(l, t, 0));
			sprite.vertices[1].position.set(LN_WRITE_V3(r, t, 0));
			sprite.vertices[2].position.set(LN_WRITE_V3(l, b, 0));
			sprite.vertices[3].position.set(LN_WRITE_V3(r, b, 0));
			break;
		}
#undef LN_WRITE_V3
	}
	// 2D の場合の頂点座標
	else
	{
		Vector2 origin(-center);
		sprite.vertices[0].position.set(origin.x, origin.y, 0);
		sprite.vertices[1].position.set(origin.x + size.x, origin.y, 0);
		sprite.vertices[2].position.set(origin.x, origin.y + size.y, 0);
		sprite.vertices[3].position.set(origin.x + size.x, origin.y + size.y, 0);
	}

	const Vector3& worldPoint = transform.position();

	Matrix actualTransform;
	{

		// ビルボード
		if (billboardType == BillboardType::ToCameraPoint)
		{
			Vector3 f = Vector3::normalize(m_viewPosition - worldPoint);
			Vector3 r = Vector3::normalize(Vector3::cross(Vector3::UnitY, f));
			Vector3 u = Vector3::cross(f, r);
			actualTransform = Matrix(
				r.x, r.y, r.z, 0.0f,
				u.x, u.y, u.z, 0.0f,
				f.x, f.y, f.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (billboardType == BillboardType::ToScreen)
		{
			// ↑がカメラ位置を基準にするのに対し、こちらはビュー平面に垂直に交差する点を基準とする。

			// ビュー平面との距離
			float d = Vector3::dot(worldPoint - m_viewPosition, m_viewDirection);

			// left-hand coord
			Vector3 f = Vector3::normalize(m_viewDirection * d);
			Vector3 r = Vector3::normalize(Vector3::cross(Vector3::UnitY, f));
			Vector3 u = Vector3::cross(f, r);
			actualTransform = Matrix(
				r.x, r.y, r.z, 0.0f,
				u.x, u.y, u.z, 0.0f,
				f.x, f.y, f.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}
		// ビルボード・Y 軸のみに適用
		else if (billboardType == BillboardType::RotY)
		{
			LN_NOTIMPLEMENTED();

			//if (m_viewDirection.x > 0.0f)
			//{
			//	rotMat.rotateY(-atanf(m_viewDirection.z / m_viewDirection.x) + Math::PI / 2);
			//}
			//else if (m_viewDirection.x == 0.0f)
			//{
			//	//D3DXMatrixIdentity(&matWorld); // 0除算を防ぐため
			//}
			//else
			//{
			//	rotMat.rotateY(-atanf(m_viewDirection.z / m_viewDirection.x) - Math::PI / 2);
			//}
		}
		// ビルボードではない
		else
		{
			actualTransform = transform.getRotationMatrix();
		}

		actualTransform.translate(worldPoint);
	}

	// 座標変換
	sprite.vertices[0].position.transformCoord(actualTransform);
	sprite.vertices[1].position.transformCoord(actualTransform);
	sprite.vertices[2].position.transformCoord(actualTransform);
	sprite.vertices[3].position.transformCoord(actualTransform);

	// 色
	sprite.vertices[0].color = color;
	sprite.vertices[1].color = color;
	sprite.vertices[2].color = color;
	sprite.vertices[3].color = color;

	// テクスチャ
	if (Math::nearEqual(m_state.textureRealSize.width, 0) || Math::nearEqual(m_state.textureRealSize.height, 0))
	{
		sprite.vertices[0].uv.x = 0;
		sprite.vertices[0].uv.y = 0;
		sprite.vertices[1].uv.x = 1;
		sprite.vertices[1].uv.y = 0;
		sprite.vertices[2].uv.x = 0;
		sprite.vertices[2].uv.y = 1;
		sprite.vertices[3].uv.x = 1;
		sprite.vertices[3].uv.y = 1;
	}
	else
	{
		Vector2 texSizeInv(1.0f / m_state.textureRealSize.width, 1.0f / m_state.textureRealSize.height);
		Rect sr(srcRect);
		float l = sr.x * texSizeInv.x;
		float t = sr.y * texSizeInv.y;
		float r = (sr.x + sr.width) * texSizeInv.x;
		float b = (sr.y + sr.height) * texSizeInv.y;
		sprite.vertices[0].uv.x = l;
		sprite.vertices[0].uv.y = t;
		sprite.vertices[1].uv.x = r;
		sprite.vertices[1].uv.y = t;
		sprite.vertices[2].uv.x = l;
		sprite.vertices[2].uv.y = b;
		sprite.vertices[3].uv.x = r;
		sprite.vertices[3].uv.y = b;
	}

	// カメラからの距離をソート用Z値にする場合
	if (m_state.sortingBasis == SortingDistanceBasis::ViewPont) {
		sprite.depth = (m_viewPosition - worldPoint).lengthSquared();
	}
	else {
		sprite.depth = worldPoint.z;
	}

	m_spriteDataList.add(sprite);
}

// Z 値の大きい方から小さい方へソートする比較
class InternalSpriteRenderer::SpriteCmpDepthBackToFront
{
public:
	List<SpriteData>* spriteList;

	bool operator()(int l_, int r_)
	{
		const SpriteData& lsp = spriteList->at(l_);
		const SpriteData& rsp = spriteList->at(r_);

		if (lsp.priority == rsp.priority)
		{
			return lsp.depth > rsp.depth;
		}
		return lsp.priority < rsp.priority;
	}
};

// Z 値の小さい方から大きい方へソートする比較
class InternalSpriteRenderer::SpriteCmpDepthFrontToBack
{
public:
	List<SpriteData>* spriteList;

	bool operator()(int l_, int r_)
	{
		const SpriteData& lsp = spriteList->at(l_);
		const SpriteData& rsp = spriteList->at(r_);

		if (lsp.priority == rsp.priority)
		{
			return lsp.depth < rsp.depth;
		}
		return lsp.priority < rsp.priority;
	}
};

void InternalSpriteRenderer::flush()
{
	int spriteCount = m_spriteDataList.size();
	if (spriteCount == 0) {
		return;
	}

	// Allocate vertex buffer and index buffer, if needed.
	prepareBuffers(m_spriteDataList.size());

	// Initialize sprite index list.
	for (int i = 0; i < spriteCount; ++i) {
		m_spriteIndexList[i] = i;
	}

	// Sort of sprite index.
	{
		if (m_state.sortMode == SpriteSortMode::DepthBackToFront)
		{
			SpriteCmpDepthBackToFront cmp;
			cmp.spriteList = &m_spriteDataList;
			std::stable_sort(m_spriteIndexList.begin(), m_spriteIndexList.begin() + spriteCount, cmp);
		}
		else if (m_state.sortMode == SpriteSortMode::DepthFrontToBack)
		{
			SpriteCmpDepthFrontToBack cmp;
			cmp.spriteList = &m_spriteDataList;
			std::stable_sort(m_spriteIndexList.begin(), m_spriteIndexList.begin() + spriteCount, cmp);
		}
	}

	// Copy vertex data.
	Vertex* vb = static_cast<Vertex*>(m_vertexBuffer->map());
	for (int iSprite = 0, iVertex = 0; iSprite < spriteCount; iSprite++)
	{
		int iData = m_spriteIndexList[iSprite];
		memcpy(&vb[iVertex], m_spriteDataList[iData].vertices, sizeof(SpriteData::vertices));
		iVertex += 4;
	}
	m_vertexBuffer->unmap();

	// Render
	m_device->setVertexDeclaration(m_vertexDeclaration);
	m_device->setVertexBuffer(0, m_vertexBuffer);
	m_device->setIndexBuffer(m_indexBuffer);
	m_device->drawPrimitiveIndexed(PrimitiveType::TriangleList, 0, spriteCount * 2);

	// Cleanup
	clear();
}

void InternalSpriteRenderer::clear()
{
	m_spriteDataList.clear();
}

void InternalSpriteRenderer::prepareBuffers(int spriteCount)
{
	if (m_buffersReservedSpriteCount < spriteCount)
	{
		size_t vertexCount = spriteCount * 4;
		if (LN_ENSURE(vertexCount < 0xFFFF)) {
			return;
		}

		size_t vertexBufferSize = sizeof(Vertex) * vertexCount;
		m_vertexBuffer = m_device->createVertexBuffer(GraphicsResourceUsage::Dynamic, vertexBufferSize, nullptr);

		size_t indexBufferSize = spriteCount * 6;
		std::vector<size_t> indexBuf(sizeof(uint16_t) * indexBufferSize, false);
		uint16_t* ib = (uint16_t*)indexBuf.data();
		int idx = 0;
		int i2 = 0;
		for (int i = 0; i < spriteCount; ++i)
		{
			i2 = i * 6;
			idx = i * 4;
			ib[i2 + 0] = idx;
			ib[i2 + 1] = idx + 1;
			ib[i2 + 2] = idx + 2;
			ib[i2 + 3] = idx + 2;
			ib[i2 + 4] = idx + 1;
			ib[i2 + 5] = idx + 3;
		}
		m_indexBuffer = m_device->createIndexBuffer(
			GraphicsResourceUsage::Dynamic, IndexBufferFormat::UInt16,
			spriteCount * 6, ib);

		m_spriteIndexList.resize(spriteCount);

		m_buffersReservedSpriteCount = spriteCount;
	}
}

//==============================================================================
// SpriteRenderFeature

SpriteRenderFeature::SpriteRenderFeature()
	: m_manager(nullptr)
	, m_state()
	, m_internal()
{
}

void SpriteRenderFeature::initialize(RenderingManager* manager)
{
	RenderFeature::initialize();
	m_manager = manager;
	m_internal = makeRef<InternalSpriteRenderer>();
	m_internal->initialize(manager);
}

void SpriteRenderFeature::setSortInfo(
	SpriteSortMode sortMode,
	SortingDistanceBasis sortingBasis)
{
	m_state.sortMode = sortMode;
	m_state.sortingBasis = sortingBasis;
}

void SpriteRenderFeature::setState(
	const Matrix& viewMatrix,
	const Matrix& projMatrix,
	const Size& textureRealSize)
{
	m_state.viewMatrix = viewMatrix;
	m_state.projMatrix = projMatrix;
	m_state.textureRealSize = textureRealSize;

	GraphicsManager* manager = m_manager->graphicsManager();
	LN_ENQUEUE_RENDER_COMMAND_2(
		SpriteRenderFeature_setState, manager,
		InternalSpriteRenderer*, m_internal,
		InternalSpriteRenderer::State, m_state,
		{
			m_internal->setState(m_state);
		});
}

void SpriteRenderFeature::drawRequest(
	const Matrix& transform,
	const Vector2& size,
	const Vector2& anchorRatio,
	const Rect& srcRect,
	const Color& color,
	SpriteBaseDirection baseDirection,
	BillboardType billboardType)
{
	GraphicsManager* manager = m_manager->graphicsManager();
	LN_ENQUEUE_RENDER_COMMAND_8(
		SpriteRenderFeature_drawRequest, manager,
		InternalSpriteRenderer*, m_internal,
		Matrix, transform,
		Vector2, size,
		Vector2, anchorRatio,
		Rect, srcRect,
		Color, color,
		SpriteBaseDirection, baseDirection,
		BillboardType, billboardType,
		{
			m_internal->drawRequest(transform, size, anchorRatio, srcRect, color, baseDirection, billboardType);
		});
}

void SpriteRenderFeature::flush()
{
	GraphicsManager* manager = m_manager->graphicsManager();
	LN_ENQUEUE_RENDER_COMMAND_1(
		SpriteRenderFeature_flush, manager,
		InternalSpriteRenderer*, m_internal,
		{
			m_internal->flush();
		});
}

} // namespace detail
} // namespace ln
