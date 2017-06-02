﻿
#pragma once

#include "../Graphics/Bitmap.h"
#include "Common.h"

LN_NAMESPACE_BEGIN
LN_NAMESPACE_GRAPHICS_BEGIN
class Texture;

/**
	@brief		
*/
class Utils
{
public:
	static IndexBufferFormat GetIndexBufferFormat(int indexCount);
	static int GetIndexStride(IndexBufferFormat format) { return (format == IndexBufferFormat_UInt16) ? 2 : 4; }
	static int GetIndexBufferSize(IndexBufferFormat format, int indexCount) { return GetIndexStride(format) * indexCount; }

	static int GetTextureFormatByteCount(TextureFormat format);
	static PixelFormat TranslatePixelFormat(TextureFormat format);
	
	static void SaveScreenShot(const TCHAR* filePath);
	static void WaitSaveScreenShot();

	static bool EqualsTexture(Texture* texture1, Texture* texture2);
};

LN_NAMESPACE_GRAPHICS_END
LN_NAMESPACE_END
