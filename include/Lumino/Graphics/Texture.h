﻿
#pragma once
#include "Common.h"
#include "../Graphics/Bitmap.h"
#include "GraphicsResourceObject.h"

LN_NAMESPACE_BEGIN
LN_NAMESPACE_GRAPHICS_BEGIN

/**
	@brief		テクスチャのクラスです。
*/
class Texture
	: public GraphicsResourceObject
{
	LN_TR_REFLECTION_TYPEINFO_DECLARE();
public:

	/**
		@brief		テクスチャのサイズを取得します。
		@return		テクスチャのサイズ (ピクセル単位)
	*/
	const Size& GetSize() const;

	/**
		@brief		テクスチャの実サイズを取得します。
		@return		テクスチャの実サイズ (ピクセル単位)
	*/
	const Size& GetRealSize() const;

	/**
		@brief		テクスチャのピクセルフォーマットを取得します。
	*/
	TextureFormat GetFormat() const;

protected:
	Texture();
	virtual ~Texture();

LN_INTERNAL_ACCESS:
	Driver::ITexture* GetDeviceObject() const { return m_deviceObj; }

protected:
	Driver::ITexture*	m_deviceObj;
	Size				m_size;
	TextureFormat		m_format;
};

/**
	@brief		テクスチャのクラスです。
*/
class Texture2D
	: public Texture
{
	LN_TR_REFLECTION_TYPEINFO_DECLARE();
public:

	/**
		@brief		指定したサイズのテクスチャを作成します。
		@param[in]	size		: テクスチャサイズ (ピクセル単位)
		@param[in]	mipLevels	: ミップマップレベル (0 を指定すると、1x1 までのすべてのミップマップテクスチャを作成する)
		@param[in]	format		: テクスチャのピクセルフォーマット
	*/
	static Texture2DPtr Create(const Size& size, TextureFormat format = TextureFormat_R8G8B8A8, int mipLevels = 1);

	/**
		@brief		ファイルからテクスチャを作成します。
		@param[in]	filePath	: ファイルパス
		@param[in]	mipLevels	: ミップマップレベル (0 を指定すると、1x1 までのすべてのミップマップテクスチャを作成する)
		@param[in]	format		: テクスチャのピクセルフォーマット
	*/
	static Texture2DPtr Create(const TCHAR* filePath, TextureFormat format = TextureFormat_R8G8B8A8, int mipLevels = 1);

	/**
		@brief		画像ファイルデータのストリームを指定してテクスチャを作成します。
		@param[in]	stream		: 画像ファイルデータのストリーム
		@param[in]	mipLevels	: ミップマップレベル (0 を指定すると、1x1 までのすべてのミップマップテクスチャを作成する)
		@param[in]	format		: テクスチャのピクセルフォーマット
	*/
	static Texture2DPtr Create(Stream* stream, TextureFormat format = TextureFormat_R8G8B8A8, int mipLevels = 1);

	/**
		@brief		メモリ上に展開された画像ファイルデータからテクスチャを作成します。
		@param[in]	data		: 画像ファイルデータの先頭アドレス
		@param[in]	size		: データのバイト数
		@param[in]	mipLevels	: ミップマップレベル (0 を指定すると、1x1 までのすべてのミップマップテクスチャを作成する)
		@param[in]	format		: テクスチャのピクセルフォーマット
	*/
	static Texture2DPtr Create(const void* data, size_t size, TextureFormat format = TextureFormat_R8G8B8A8, int mipLevels = 1);

public:
	
	/*
		@brief		
	*/
	void SetSubData(const Point& offset, Bitmap* bitmap);
	void SetSubData(const Point& offset, const void* data);

	Bitmap* Lock();
	void Unlock();
	//Device::ITexture* GetDeviceObject() const { return m_deviceObj; }

LN_PROTECTED_INTERNAL_ACCESS:
	Texture2D();
	//Texture(GraphicsManager* manager, const Size& size, TextureFormat format, int mipLevels, Bitmap* primarySurface);
	//Texture(GraphicsManager* manager, Stream* stream, TextureFormat format, int mipLevels);
	////Texture(GraphicsManager* manager, Driver::ITexture* deviceObj, Bitmap* primarySurface = NULL);
	//Texture(GraphicsManager* manager, bool isDefaultBackBuffer);
	virtual ~Texture2D();
	void FlushPrimarySurface();
	virtual void OnChangeDevice(Driver::IGraphicsDevice* device);

LN_INTERNAL_ACCESS:
	void CreateCore(GraphicsManager* manager, const Size& size, TextureFormat format, int mipLevels, Bitmap* primarySurface);
	void CreateCore(GraphicsManager* manager, const TCHAR* filePath, TextureFormat format, int mipLevels);
	void CreateCore(GraphicsManager* manager, Stream* stream, TextureFormat format, int mipLevels);
	void CreateCore(GraphicsManager* manager, bool isDefaultBackBuffer);
	Driver::ITexture* GetDeviceObject() const { return m_deviceObj; }
	void AttachDefaultBackBuffer(Driver::ITexture* deviceObj);
	void DetachDefaultBackBuffer();

protected:
	friend struct SetRenderTargetCommand;	// TODO: ダサイ
	friend struct SetDepthBufferCommand;
	friend struct ReadLockTextureCommand;
	friend struct ReadUnlockTextureCommand;
	friend struct PresentCommand;	// TODO
	friend class ShaderVariable;
	friend class SwapChain;
	int					m_mipLevels;
	Bitmap*				m_primarySurface;
	bool				m_isPlatformLoaded;
	bool				m_isDefaultBackBuffer;
	//bool				m_primarySurfaceModified;

	friend class Helper;
};


/**
	@brief		レンダリングターゲットのクラスです。
*/
class RenderTarget
	: public Texture
{
public:

	/**
		@brief		レンダリングターゲットを作成します。
		@param[in]	size		: レンダリングターゲットのサイズ (ピクセル単位)
		@param[in]	mipLevels	: ミップマップレベル (0 を指定すると、1x1 までのすべてのミップマップテクスチャを作成する)
		@param[in]	format		: テクスチャのピクセルフォーマット
	*/
	static Texture* Create(const Size& size, int mipLevels = 1, TextureFormat format = TextureFormat_R8G8B8A8);

LN_INTERNAL_ACCESS:
	RenderTarget();
	void CreateImpl(GraphicsManager* manager, const Size& size, int mipLevels, TextureFormat format);

protected:
	virtual ~RenderTarget();
	virtual void OnChangeDevice(Driver::IGraphicsDevice* device);

private:
	//Size			m_size;
	int				m_mipLevels;
	//TextureFormat	m_format;
};

/**
	@brief		深度バッファのクラスです。
*/
class DepthBuffer
	: public Texture
{
public:

	/**
		@brief		深度バッファを作成します。
		@param[in]	size		: テクスチャのサイズ (ピクセル単位)
		@param[in]	format		: ピクセルフォーマット
	*/
	static Texture* Create(const Size& size, TextureFormat format = TextureFormat_D24S8);

LN_INTERNAL_ACCESS:
	DepthBuffer();
	void CreateImpl(GraphicsManager* manager, const Size& size, TextureFormat format);

protected:
	virtual ~DepthBuffer();
	virtual void OnChangeDevice(Driver::IGraphicsDevice* device);

private:
	//Size			m_size;
	//TextureFormat	m_format;
};

LN_NAMESPACE_GRAPHICS_END
LN_NAMESPACE_END
