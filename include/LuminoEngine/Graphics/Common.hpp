﻿#pragma once

namespace ln {

enum class GraphicsAPI
{
	OpenGL,
};

enum class ClearFlags
{
	None	= 0x0000,
	Color	= 0x0001,					///< カラーバッファをクリアします。
	Depth	= 0x0002,					///< 深度バッファをクリアします。
	Stencil = 0x0004,					///< ステンシルバッファをクリアします。
	All		= Color | Depth | Stencil,	///< 全てのバッファをクリアします。
};
LN_FLAGS_OPERATORS(ClearFlags);

/** グラフィックスリソースの使用方法 */
enum class GraphicsResourceUsage : uint8_t
{
	/** 頻繁に更新を行わないリソース */
	Static,

	/** 頻繁に更新を行うリソース */
	Dynamic,
};

/** グラフィックスリソースの管理方法 */
enum class GraphicsResourcePool
{
	/** デバイス変更時に内容を復元しません。*/
	None,

	/** デバイス変更時に内容を復元します。*/
	Managed,
};

/** グラフィックスリソースのマップ方法 */
enum class MapMode
{
	/** 読み取りモード */
	Read,

	/** 書き込みモード */
	Write,
};

/** 頂点宣言の要素の型 */
enum class VertexElementType
{
	/** Unknown */
	Unknown,

	/** float */
	Float1,

	/** float[2] (Vector2) */
	Float2,

	/** float[3] (Vector3) */
	Float3,

	/** float[4] (Vector4) */
	Float4,

	/** uint8_t[4] */
	Ubyte4,

	/** 32ビット色コード (使用非推奨。DirectX と OpenGL ではバイトオーダが異なる。DXは0xAARRGGBB、GLは0xAABBGGRRで、GLES ではオーダーの変更ができない) */
	Color4,

	/** short[2] */
	Short2,

	/** short[4] */
	Short4,
};

/** 頂点宣言の要素の用途 */
enum class VertexElementUsage
{
	Unknown,
	Position,
	Normal,
	Color,
	TexCoord,
	PointSize,	// MME との互換性のために残している。
	BlendIndices,
	BlendWeight,
};

/** 頂点宣言の1要素 */
struct VertexElement
{
	/** ストリーム番号 */
	uint32_t StreamIndex;

	/** 要素の型 */
	VertexElementType Type;

	/** 要素の用途 */
	VertexElementUsage Usage;

	/** UsageIndex */
	uint32_t UsageIndex;
};

/** インデックスバッファのフォーマット */
enum class IndexBufferFormat
{
	/** 16 bit */
	UInt16,

	/** 32 bit */
	UInt32,
};

/** ピクセルフォーマット */
LN_ENUM()
enum class PixelFormat
{
	/** Unknown */
	Unknown,

	A1,
	A8,

	RGBA32,

	R32G32B32A32Float,
};

/** テクスチャのピクセルフォーマット */
LN_ENUM()
enum class TextureFormat
{
	/** Unknown */
	Unknown,

	/** 32 ビットのアルファ付きフォーマット (GPUネイティブフォーマット。D3D_FMT_A8B8G8R8, DXGI_FORMAT_R8G8B8A8_UNORM) */
	RGBA32,

	/** 32 ビットのアルファ無しフォーマット */
	RGBX32,

	/** 64 ビットの浮動小数点フォーマット */
	R16G16B16A16Float,

	/** 128 ビットの浮動小数点フォーマット */
	R32G32B32A32Float,

	/** 16 ビットの浮動小数点フォーマット */
	R16Float,

	/** 32 ビットの浮動小数点フォーマット */
	R32Float,

	/** 32 ビットの整数フォーマット */
	R32UInt,
};

/** 深度バッファのフォーマット */
LN_ENUM()
enum class DepthBufferFormat
{
	/** S8 32 ビット*/
	D24S8,
};

/** 描画プリミティブの種類 */
enum class PrimitiveType
{
	TriangleList,
	TriangleStrip,
	TriangleFan,
	LineList,
	LineStrip,
	PointList,
};

enum class ShaderCompilationResultLevel
{
	Success,
	Warning,
	Error,
};

struct ShaderCompilationDiag
{
	ShaderCompilationResultLevel level;
	std::string message;
};

enum class ShaderVariableType
{
	Unknown,
	Bool,
	BoolArray,
	Int,
	Float,
	FloatArray,
	Vector,
	VectorArray,
	Matrix,
	MatrixArray,
	Texture,
	Pointer,

	/*
	*	Texture は、DirectX9HLSL では texture 型を表し、GLSL では sampler 型を表す。
	*	GLSL では sampler しか無いが、DirectX9HLSL では texture と sampler の2種類が存在する。
	*/
};

class GraphicsHelper
{
public:
	static PixelFormat translateToPixelFormat(TextureFormat format);
	static TextureFormat translateToTextureFormat(PixelFormat format);
};










/// テクスチャフィルタ
enum class TextureFilterMode
{
	Point = 0,		///< 補間を行わない
	Linear,			///< 補間を行う
};

/// テクスチャアドレッシング
enum class TextureAddressMode
{
	Repeat = 0,		///< 繰り返し
	Clamp,			///< 境界のピクセルが引き延ばされる
};

struct SamplerStateData
{
	TextureFilterMode filter;
	TextureAddressMode address;

	static const SamplerStateData defaultState;
};

namespace detail {

using ShaderRefrectionParameterType = ShaderVariableType;
//enum class ShaderRefrectionParameterType
//{
//	Unknown,
//	Bool,
//	BoolArray,
//	Int,
//	Float,
//	FloatArray,
//	Vector,
//	VectorArray,
//	Matrix,
//	MatrixArray,
//	Texture,
//};

// rows, columns はデータレイアウトとしての領域サイズ。
struct ShaderUniformTypeDesc
{
	ShaderRefrectionParameterType type;
	int rows;
	int columns;
	int elements;

	size_t offset;
	size_t size;
	size_t arrayStride;
	size_t matrixStride;

	static bool equals(const ShaderUniformTypeDesc& a, const ShaderUniformTypeDesc& b)
	{
		return
			a.type == b.type &&
			a.rows == b.rows &&
			a.columns == b.columns &&
			a.elements == b.elements;
	}
};

} // namespace detail
} // namespace ln