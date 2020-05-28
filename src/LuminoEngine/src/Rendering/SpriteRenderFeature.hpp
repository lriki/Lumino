﻿#pragma once
#include <LuminoCore/Base/EnumFlags.hpp>
#include <LuminoEngine/Graphics/GeometryStructs.hpp>
#include <LuminoEngine/Graphics/ColorStructs.hpp>
#include <LuminoEngine/Graphics/VertexLayout.hpp>
#include <LuminoEngine/Rendering/RenderFeature.hpp>
#include <LuminoEngine/Rendering/Vertex.hpp>
#include "../Graphics/GraphicsDeviceContext.hpp"
#include "RenderStage.hpp"

namespace ln {
class VertexBuffer;
class IndexBuffer;
class Texture;
namespace detail {

// スプライトのソート方法
enum class SpriteSortMode
{
	None,				// ソートしない
	DepthBackToFront,	// Z値が大きいものが先に描画されるようにソートする (アルファブレンド使用時の推奨)
	DepthFrontToBack,	// Z値が小さいものが先に描画されるようにソートする (SpriteSortMode_DepthBackToFront と同時には使えない)
};

// オブジェクトのソートの基準
enum class SortingDistanceBasis
{
	RawZ,		// オブジェクトの Z 値
	ViewPont,	// オブジェクトの位置と視点との距離
};

// 実際の描画を行う内部クラス。
// レンダリングスレッド上で動作する。
class InternalSpriteRenderer
	: public RefObject
{
public:
	struct State
	{
		Matrix viewMatrix;
		Matrix projMatrix;
		SpriteSortMode sortMode = SpriteSortMode::DepthBackToFront;
		SortingDistanceBasis sortingBasis = SortingDistanceBasis::ViewPont;

		bool operator==(const State& other) const
		{
			return
				viewMatrix == other.viewMatrix &&
				projMatrix == other.projMatrix &&
				sortMode == other.sortMode &&
				sortingBasis == other.sortingBasis;
		}
	};

	InternalSpriteRenderer();
	void init(RenderingManager* manager);

	void setState(const State& state);

	void drawRequest(
		const Matrix& transform,
		const Vector2& size,
		const Vector2& anchorRatio,
		const Rect& srcRect,
		const Color& color,
		SpriteBaseDirection baseDir,
		BillboardType billboardType,
        SpriteFlipFlags flipFlags);

	void flush(ICommandList* context);
	void clear();

private:
	class SpriteCmpDepthBackToFront;		// Z 値の大きい方から小さい方へソートする比較
	class SpriteCmpDepthFrontToBack;		// Z 値の小さい方から大きい方へソートする比較

	struct SpriteData
	{
		Vertex vertices[4];
		int priority;	// 優先度 (大きい方が後から描画される =手前)
		float depth;	// ソートに使われる Z 値 (大きいほど遠い)
	};

	void prepareBuffers(IGraphicsDevice* context, int spriteCount);

	State m_state;
	Matrix m_viewInverseMatrix;
	Vector3 m_viewDirection;
	Vector3 m_viewPosition;
	List<SpriteData> m_spriteDataList;
	List<int> m_spriteIndexList;

	//IGraphicsDevice* m_device;
	Ref<IVertexDeclaration> m_vertexDeclaration;
	Ref<IVertexBuffer> m_vertexBuffer;
	Ref<IIndexBuffer> m_indexBuffer;
	int m_buffersReservedSpriteCount;
};

class SpriteRenderFeature
	: public RenderFeature
{
public:
	void setSortInfo(
		SpriteSortMode sortMode,
		SortingDistanceBasis sortingBasis);

    // srcRect は UV 座標系上の値を設定する。 (通常0～1)
    // 以前は 2D メインな Sprite なのでピクセル単位で指定していたが、
    // 考え方として他の RenderFeature と同様に「最終的な描画に使うメッシュを作る」方針で統一したい。
	void drawRequest(
		GraphicsContext* context,
		const Matrix& transform,
		const Vector2& size,
		const Vector2& anchorRatio,
		const Rect& srcRect,
		const Color& color,
		SpriteBaseDirection baseDirection,
		BillboardType billboardType,
        SpriteFlipFlags flipFlags);

    virtual void onActiveRenderFeatureChanged(const detail::CameraInfo& mainCameraInfo) override;
	virtual void submitBatch(GraphicsContext* context, detail::RenderFeatureBatchList* batchList) override;
	virtual void renderBatch(GraphicsContext* context, RenderFeatureBatch* batch) override;

    static void makeRenderSizeAndSourceRectHelper(Texture* texture, const Size& size, const Rect& sourceRect, Size* outSize, Rect* outSourceRect);

    // TODO:
    // drawElementTransformNegate

LN_CONSTRUCT_ACCESS:
	SpriteRenderFeature();
	void init(RenderingManager* manager);

private:
	RenderingManager* m_manager;
	InternalSpriteRenderer::State m_state;
	Ref<InternalSpriteRenderer> m_internal;
    bool m_stateChanged;
};


class SpriteRenderFeature2 : public RenderFeature
{
public:
	// srcRect は UV 座標系上の値を設定する。 (通常0～1)
	// 以前は 2D メインな Sprite なのでピクセル単位で指定していたが、
	// 考え方として他の RenderFeature と同様に「最終的な描画に使うメッシュを作る」方針で統一したい。
	RequestBatchResult drawRequest(
		detail::RenderFeatureBatchList* batchList,
		GraphicsContext* context,
		const Matrix& transform,
		const Vector2& size,
		const Vector2& anchorRatio,
		const Rect& srcRect,
		const Color& color,
		SpriteBaseDirection baseDirection,
		BillboardType billboardType,
		SpriteFlipFlags flipFlags);

	virtual bool drawElementTransformNegate() const override { return true; }
	virtual void onActiveRenderFeatureChanged(const detail::CameraInfo& mainCameraInfo) override;
	virtual void submitBatch(GraphicsContext* context, detail::RenderFeatureBatchList* batchList) override;
	virtual void renderBatch(GraphicsContext* context, RenderFeatureBatch* batch) override;

LN_CONSTRUCT_ACCESS:
	SpriteRenderFeature2();
	void init(RenderingManager* manager);

private:
	struct BatchData
	{
		int spriteOffset;
		int spriteCount;
	};

	class Batch : public RenderFeatureBatch
	{
	public:
		BatchData data;
	};

	void prepareBuffers(GraphicsContext* context, int spriteCount);

	RenderingManager* m_manager;
	Matrix m_viewMatrix;
	Matrix m_projMatrix;
	Matrix m_viewInverseMatrix;
	Vector3 m_viewDirection;
	Vector3 m_viewPosition;

	// sprite-batching
	Ref<VertexLayout> m_vertexLayout;
	Ref<VertexBuffer> m_vertexBuffer;
	Ref<IndexBuffer> m_indexBuffer;
	int m_buffersReservedSpriteCount;
	BatchData m_batchData;
	Vertex* m_mappedVertices;
};

} // namespace detail
} // namespace ln

