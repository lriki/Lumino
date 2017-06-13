﻿
#pragma once
#include "Common.h"
#include "GraphicsResourceObject.h"

LN_NAMESPACE_BEGIN
class PlatformWindow;

LN_NAMESPACE_GRAPHICS_BEGIN

/**
	@brief		スワップチェインのクラスです。
*/
class SwapChain
	: public GraphicsResourceObject
{
public:
	virtual ~SwapChain();
	virtual void Dispose() override;

public:

	/**
		@brief	バックバッファのレンダリングターゲットを取得します。
	*/
	RenderTargetTexture* getBackBuffer();



	/**
		@brief	バックバッファのレンダリング結果をフロントバッファに転送します。
	*/
	void present();

LN_INTERNAL_ACCESS:
	SwapChain();
	void initializeDefault(detail::GraphicsManager* manager);
	void initializeSub(detail::GraphicsManager* manager, PlatformWindow* window);
	virtual void onChangeDevice(Driver::IGraphicsDevice* device);

	void postInitialize();

	void MightResizeAndDeviceReset(const SizeI& newSize);
	void PresentInternal();
	void WaitForPresent();

	Driver::ISwapChain*			m_deviceObj;
	RenderingCommandList*		m_commandList;
	ConditionFlag				m_waiting;		// コマンド実行していない
	RenderTargetTexture*		m_backColorBuffer;
};

LN_NAMESPACE_GRAPHICS_END
LN_NAMESPACE_END
