﻿
#pragma once
#include "../Internal.h"
#include <Lumino/IO/FileManager.h>
#include "../../include/Lumino/Graphics/Texture.h"
#include "GraphicsManager.h"
#include <Lumino/Graphics/Utils.h>
#include "RendererImpl.h"
#include "Internal.h"
#include "RenderingCommand.h"
#include "RenderingThread.h"
#include "GraphicsHelper.h"
#include "Text/BitmapTextRenderer.h"

LN_NAMESPACE_BEGIN
LN_NAMESPACE_GRAPHICS_BEGIN

//=============================================================================
// Texture
//=============================================================================
LN_TR_REFLECTION_TYPEINFO_IMPLEMENT(Texture, GraphicsResourceObject);

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture::Texture()
	: m_deviceObj(NULL)
	, m_size()
	, m_format(TextureFormat_Unknown)
	, m_primarySurface(NULL)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture::~Texture()
{
	LN_SAFE_RELEASE(m_deviceObj);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
const Size& Texture::GetSize() const
{
	return m_deviceObj->GetSize();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int Texture::GetWidth() const
{
	return m_deviceObj->GetSize().width;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int Texture::GetHeight() const
{
	return m_deviceObj->GetSize().height;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
const Size& Texture::GetRealSize() const
{
	return m_deviceObj->GetRealSize();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
TextureFormat Texture::GetFormat() const
{
	return m_deviceObj->GetTextureFormat();
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Bitmap* Texture::Lock()
{
	// Deferred
	if (m_manager->GetRenderingType() == RenderingType::Deferred)
	{
		if (m_deviceObj->GetTextureType() == Driver::TextureType_Normal)
		{
			return m_primarySurface;
		}
		else if (m_deviceObj->GetTextureType() == Driver::TextureType_RenderTarget)
		{
			RenderingCommandList* cmdList = m_manager->GetRenderer()->m_primaryCommandList;
			cmdList->AddCommand<ReadLockTextureCommand>(this);

			// ここでたまっているコマンドをすべて実行する。
			// ReadLockTexture コマンドが実行されると、m_lockedBitmap に
			// ロックしたビットマップがセットされる。
			m_manager->GetRenderingThread()->PushRenderingCommand(cmdList);
			cmdList->WaitForIdle();

			return m_primarySurface;
		}
		return NULL;
	}
	// Immediate
	else
	{
		return m_deviceObj->Lock();
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Texture::Unlock()
{
	// Deferred
	if (m_manager->GetRenderingType() == RenderingType::Deferred)
	{
		RenderingCommandList* cmdList = m_manager->GetRenderer()->m_primaryCommandList;
		if (m_deviceObj->GetTextureType() == Driver::TextureType_Normal)
		{
			// 変更済みフラグを ON にしておく。
			// あとで本当に使うタイミング (=シェーダパスのApply時) になったら FlushPrimarySurface() が呼ばれる
			//m_primarySurfaceModified = true;

			// TODO: 遅延転送
			//cmdList->SetTextureSubData(m_deviceObj, m_primarySurface);
			//cmdList->AddCommand<SetTextureSubDataCommand>(m_deviceObj, m_primarySurface);
			//SetTextureSubDataCommand::AddCommand(cmdList, m_deviceObj, m_primarySurface);
			cmdList->AddCommand<SetSubDataTextureCommand>(
				m_deviceObj, Point(0, 0), m_primarySurface->GetBitmapBuffer()->GetConstData(), m_primarySurface->GetBitmapBuffer()->GetSize(), m_deviceObj->GetSize());
		}
		else if (m_deviceObj->GetTextureType() == Driver::TextureType_RenderTarget)
		{
			cmdList->AddCommand<ReadUnlockTextureCommand>(this);
			//ReadUnlockTextureCommand::AddCommand(cmdList, this);
			//cmdList->ReadUnlockTexture(this);
			m_manager->GetRenderingThread()->PushRenderingCommand(cmdList);
			cmdList->WaitForIdle();
		}
	}
	// Immediate
	else
	{
		m_deviceObj->Unlock();
	}
}

//=============================================================================
// Texture2D
//=============================================================================
LN_TR_REFLECTION_TYPEINFO_IMPLEMENT(Texture2D, Texture);

//static GraphicsManager* GetManager()
//{
//	assert(GraphicsManager::Instance != NULL);
//	return GraphicsManager::Instance;
//}
//static Driver::IGraphicsDevice* GetDevice()
//{
//	assert(GraphicsManager::Instance != NULL);
//	return GraphicsManager::Instance->GetGraphicsDevice();
//}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture2DPtr Texture2D::Create(int width, int height, TextureFormat format, int mipLevels)
{
	return Create(Size(width, height), format, mipLevels);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture2DPtr Texture2D::Create(const Size& size, TextureFormat format, int mipLevels)
{
	// ロック用のビットマップを作る
	RefPtr<Bitmap> bitmap(LN_NEW Bitmap(size, Utils::TranslatePixelFormat(format)), false);
	RefPtr<Texture2D> tex(LN_NEW Texture2D(), false);
	tex->CreateCore(GraphicsManager::GetInstance(), size, format, mipLevels, bitmap);
	return tex;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture2DPtr Texture2D::Create(const StringRef& filePath, TextureFormat format, int mipLevels)
{
	RefPtr<Texture2D> tex(LN_NEW Texture2D(), false);
	tex->CreateCore(GraphicsManager::GetInstance(), filePath, format, mipLevels);
	return tex;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture2DPtr Texture2D::Create(Stream* stream, TextureFormat format, int mipLevels)
{
	RefPtr<Texture2D> tex(LN_NEW Texture2D(), false);
	tex->CreateCore(GraphicsManager::GetInstance(), stream, format, mipLevels);
	return tex;
	/*
	if (GetManager()->IsPlatformTextureLoading())
	{
		RefPtr<Texture2D> tex(LN_NEW Texture2D(), false);
		tex->CreateImpl(GetManager(), stream, format, mipLevels);
		tex.SafeAddRef();
		return tex;
	}

	// ビットマップを作る
	RefPtr<Bitmap> bitmap(LN_NEW Bitmap(stream), false);
	RefPtr<Texture2D> tex(LN_NEW Texture2D(), false);
	tex->CreateImpl(GetManager(), bitmap->GetSize(), format, mipLevels, bitmap);
	tex.SafeAddRef();
	return tex;
	*/
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture2DPtr Texture2D::Create(const void* data, size_t size, TextureFormat format, int mipLevels)
{
	MemoryStream stream(data, size);
	return Create(&stream, format, mipLevels);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture2D::Texture2D()
	: m_isPlatformLoaded(false)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Texture2D::CreateCore(GraphicsManager* manager, const Size& size, TextureFormat format, int mipLevels, Bitmap* primarySurface)
{
	GraphicsResourceObject::Initialize(manager);

	m_size = size;
	m_mipLevels = mipLevels;
	m_format = format;
	LN_REFOBJ_SET(m_primarySurface, primarySurface);

	// テクスチャを作る
	m_deviceObj = GraphicsManager::GetInstance()->GetGraphicsDevice()->CreateTexture(primarySurface->GetSize(), mipLevels, format, primarySurface->GetBitmapBuffer()->GetConstData());

	// ビットマップを転送する
	//Driver::IGraphicsDevice::ScopedLockContext lock(GetDevice());
	//obj->SetSubData(Point(0, 0), primarySurface->GetBitmapBuffer()->GetConstData(), primarySurface->GetBitmapBuffer()->GetSize(), primarySurface->GetSize());
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Texture2D::CreateCore(GraphicsManager* manager, const StringRef& filePath, TextureFormat format, int mipLevels)
{
	RefPtr<Stream> stream(manager->GetFileManager()->CreateFileStream(filePath), false);
	CreateCore(manager, stream, format, mipLevels);
}

//-----------------------------------------------------------------------------
// プラットフォーム依存用
//-----------------------------------------------------------------------------
void Texture2D::CreateCore(GraphicsManager* manager, Stream* stream, TextureFormat format, int mipLevels)
{
	GraphicsResourceObject::Initialize(manager);
	m_mipLevels = mipLevels;
	m_format = format;

	if (m_manager->IsPlatformTextureLoading())
	{
		m_deviceObj = GraphicsManager::GetInstance()->GetGraphicsDevice()->CreateTexturePlatformLoading(stream, mipLevels, format);
		if (m_deviceObj != NULL)
		{
			m_primarySurface = LN_NEW Bitmap(m_deviceObj->GetSize(), Utils::TranslatePixelFormat(format));
			m_size = m_deviceObj->GetSize();
		}
	}

	// プラットフォーム依存のロードが失敗したら普通の処理
	if (m_deviceObj == NULL)
	{
		RefPtr<Bitmap> bitmap(LN_NEW Bitmap(stream), false);
		CreateCore(m_manager, bitmap->GetSize(), format, mipLevels, bitmap);
	}

#if 0
	m_isPlatformLoaded = true;

	m_deviceObj = GetDevice()->CreateTexturePlatformLoading(stream, mipLevels, format);
	if (m_deviceObj != NULL)
	{
		m_primarySurface = LN_NEW Bitmap(m_deviceObj->GetSize(), Utils::TranslatePixelFormat(format));
		// TODO: m_primarySurface へ内容をフィードバックする
		//Driver::ITexture::ScopedLock lock(m_deviceObj);
		//m_primarySurface->CopyRawData(lock.GetBitmap(), m_primarySurface->GetByteCount());
		m_size = m_deviceObj->GetSize();
	}
	// TODO: 失敗したら普通の処理
#endif
}

//Texture2D::Texture(GraphicsManager* manager, Driver::ITexture* deviceObj, Bitmap* primarySurface)
//	: m_manager(manager)
//	, m_deviceObj(deviceObj)
//	, m_primarySurface(primarySurface)
//	//, m_primarySurfaceModified(false)
//	, m_isDefaultBackBuffer(false)
//{
//	LN_SAFE_ADDREF(m_primarySurface);
//	m_manager->AddResourceObject(this);
//}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture2D::~Texture2D()
{
	LN_SAFE_RELEASE(m_primarySurface);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Texture2D::Clear(const Color& color)
{
	ScopedTextureLock lock(this);
	lock.GetBitmap()->Clear(color);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Texture2D::Blt(int x, int y, Texture* srcTexture, const Rect& srcRect)
{
	ScopedTextureLock lock1(this);
	ScopedTextureLock lock2(srcTexture);
	lock1.GetBitmap()->BitBlt(x, y, lock2.GetBitmap(), srcRect, Color::White, true);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#pragma push_macro("DrawText")
#undef DrawText
void Texture2D::DrawText(const StringRef& text, const Rect& rect, Font* font, const Color& fillColor, const Color& strokeColor, int strokeThickness, TextAlignment alignment) { LN_AFX_FUNCNAME(DrawText)(text, rect, font, fillColor, strokeColor, strokeThickness, alignment); }
void Texture2D::LN_AFX_FUNCNAME(DrawText)(const StringRef& text, const Rect& rect, Font* font, const Color& fillColor, const Color& strokeColor, int strokeThickness, TextAlignment alignment)
{
	ScopedTextureLock lock(this);
	auto* r = m_manager->GetBitmapTextRenderer();
	auto* gr = r->GetTempGlyphRun();
	gr->SetFont(font);
	gr->SetText(text);
	//gr->SetTextAlignment(alignment);
	r->SetRenderArea(rect);
	r->SetTextAlignment(alignment);
	r->DrawGlyphRun(lock.GetBitmap(), gr, fillColor, strokeColor, strokeThickness);
}
#pragma pop_macro("DrawText")

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Texture2D::SetSubData(const Point& offset, Bitmap* bitmap)
{
	LN_CHECK_ARGS_RETURN(bitmap != NULL);

	// TODO: 現状、ピクセルフォーマットが一致していることが前提
	if (bitmap->GetPixelFormat() != Utils::TranslatePixelFormat(m_deviceObj->GetTextureFormat())) {
		LN_THROW(0, NotImplementedException);
	}

	LN_CALL_TEXTURE_COMMAND(SetSubData, SetSubDataTextureCommand,
		offset, bitmap->GetBitmapBuffer()->GetConstData(), bitmap->GetBitmapBuffer()->GetSize(), bitmap->GetSize());
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Texture2D::SetSubData(const Point& offset, const void* data)
{
	LN_CHECK_ARGS_RETURN(data != NULL);
	// TODO: m_primarySurface にもセット
	LN_CALL_TEXTURE_COMMAND(SetSubData, SetSubDataTextureCommand,
		offset, data, m_primarySurface->GetBitmapBuffer()->GetSize(), m_deviceObj->GetSize());
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Texture2D::OnChangeDevice(Driver::IGraphicsDevice* device)
{
	if (device == NULL)
	{
		// Immediate のときは Lock で取得する必要がある。Deferred のときは m_primarySurface が持っている。
		if (m_manager->GetRenderingType() == RenderingType::Immediate)
		{
			Driver::ITexture::ScopedLock lock(m_deviceObj);
			m_primarySurface->CopyRawData(lock.GetBitmap()->GetBitmapBuffer()->GetConstData(), m_primarySurface->GetByteCount());
			//m_primarySurface->Save(_T("test.png"));
		}
		LN_SAFE_RELEASE(m_deviceObj);
	}
	else
	{
		// この時点では描画モードにかかわらず m_primarySurface がバックアップデータを保持しているのでそれから復元する。
		m_deviceObj = device->CreateTexture(m_primarySurface->GetSize(), m_mipLevels, m_format, m_primarySurface->GetBitmapBuffer()->GetConstData());
		//m_deviceObj->SetSubData(Point(0, 0), m_primarySurface->GetBitmapBuffer()->GetConstData(), m_primarySurface->GetBitmapBuffer()->GetSize(), m_primarySurface->GetSize());
		// TODO: Create でinitialデータも渡してしまう。
	}
}


//=============================================================================
// RenderTarget
//=============================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
RenderTargetPtr RenderTarget::Create(const Size& size, TextureFormat format, int mipLevels)
{
	RefPtr<RenderTarget> tex(LN_NEW RenderTarget(), false);
	tex->CreateImpl(GraphicsManager::GetInstance(), size, mipLevels, format);
	return tex;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
RenderTarget::RenderTarget()
	: Texture()
	, m_mipLevels(0)
	, m_isDefaultBackBuffer(false)
	, m_usedCacheOnFrame(false)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RenderTarget::CreateImpl(GraphicsManager* manager, const Size& size, int mipLevels, TextureFormat format)
{
	GraphicsResourceObject::Initialize(manager);

	m_size = size;
	m_mipLevels = mipLevels;
	m_format = format;
	m_deviceObj = m_manager->GetGraphicsDevice()->CreateRenderTarget(m_size.width, m_size.height, m_mipLevels, m_format);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RenderTarget::CreateCore(GraphicsManager* manager, bool isDefaultBackBuffer)
{
	GraphicsResourceObject::Initialize(manager);
	m_deviceObj = NULL;
	//m_primarySurface = NULL;
	m_isDefaultBackBuffer = isDefaultBackBuffer;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
RenderTarget::~RenderTarget()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RenderTarget::AttachDefaultBackBuffer(Driver::ITexture* deviceObj)
{
	assert(m_isDefaultBackBuffer);
	assert(m_deviceObj == NULL);
	LN_REFOBJ_SET(m_deviceObj, deviceObj);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RenderTarget::DetachDefaultBackBuffer()
{
	assert(m_isDefaultBackBuffer);
	assert(m_deviceObj != NULL);
	LN_SAFE_RELEASE(m_deviceObj);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RenderTarget::OnChangeDevice(Driver::IGraphicsDevice* device)
{
	if (device == NULL) {
		LN_SAFE_RELEASE(m_deviceObj);
	}
	else {
		m_deviceObj = m_manager->GetGraphicsDevice()->CreateRenderTarget(m_size.width, m_size.height, m_mipLevels, m_format);
	}
}

//=============================================================================
// DepthBuffer
//=============================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Texture* DepthBuffer::Create(const Size& size, TextureFormat format)
{
	RefPtr<DepthBuffer> tex(LN_NEW DepthBuffer(), false);
	tex->CreateImpl(GraphicsManager::GetInstance(), size, format);
	tex.SafeAddRef();
	return tex;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
DepthBuffer::DepthBuffer()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DepthBuffer::CreateImpl(GraphicsManager* manager, const Size& size, TextureFormat format)
{
	GraphicsResourceObject::Initialize(manager);

	m_size = size;
	m_format = format;
	m_deviceObj = m_manager->GetGraphicsDevice()->CreateDepthBuffer(m_size.width, m_size.height, m_format);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
DepthBuffer::~DepthBuffer()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DepthBuffer::OnChangeDevice(Driver::IGraphicsDevice* device)
{
	if (device == NULL) {
		LN_SAFE_RELEASE(m_deviceObj);
	}
	else {
		m_deviceObj = m_manager->GetGraphicsDevice()->CreateDepthBuffer(m_size.width, m_size.height, m_format);
	}
}

LN_NAMESPACE_GRAPHICS_END
LN_NAMESPACE_END
