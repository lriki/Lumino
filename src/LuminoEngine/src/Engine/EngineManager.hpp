﻿#pragma once
#include <LuminoEngine/Graphics/GeometryStructs.hpp>
#include <LuminoEngine/Platform/PlatformEvent.hpp>

namespace ln {
class UIFrameWindow;

namespace detail {
class PlatformManager;
class AnimationManager;
class PhysicsManager;
class InputManager;
class AudioManager;
class AnimationManager;
class ShaderManager;
class GraphicsManager;
class MeshManager;
class RenderingManager;
class EffectManager;
class AssetManager;
class UIManager;

struct EngineSettings
{
	SizeI mainWindowSize = SizeI(640, 480);
	SizeI mainBackBufferSize = SizeI(640, 480);
	String mainWindowTitle = _T("Lumino");
};

class EngineManager
	: public RefObject
	, public IPlatforEventListener
{
public:
	EngineManager();
	virtual ~EngineManager();

	void initialize();
	void dispose();

	void initializeAllManagers();
	void initializeCommon();
	void initializePlatformManager();
	void initializeAnimationManager();
	void initializeInputManager();
	void initializeAudioManager();
	void initializePhysicsManager();
	void initializeShaderManager();
	void initializeGraphicsManager();
	void initializeMeshManager();
	void initializeRenderingManager();
	void initializeEffectManager();
	void initializeAssetManager();
	void initializeUIManager();

	bool updateUnitily();
	void updateFrame();
	void renderFrame();		// Engine 内部管理のレンダリング
	void presentFrame();	// swap。renderFrame() と分けているのは、間に コールバック以外の Engine 外部のレンダリングを許可するため
	void resetFrameDelay();
	bool isExitRequested() const { return m_exitRequested; }
	void exit();

	EngineSettings& settings() { return m_settings; }
	const Ref<PlatformManager>& platformManager() const { return m_platformManager; }
	const Ref<InputManager>& inputManager() const { return m_inputManager; }
	const Ref<ShaderManager>& shaderManager() const { return m_shaderManager; }
	const Ref<GraphicsManager>& graphicsManager() const { return m_graphicsManager; }
	const Ref<MeshManager>& meshManager() const { return m_meshManager; }
	const Ref<RenderingManager>& renderingManager() const { return m_renderingManager; }
	const Ref<UIManager>& uiManager() const { return m_uiManager; }
	const Ref<UIFrameWindow>& mainWindow() const { return m_mainWindow; }

private:
	virtual bool onPlatformEvent(const PlatformEventArgs& e) override;

	EngineSettings m_settings;

	Ref<PlatformManager>				m_platformManager;
	//Ref<AnimationManager>			m_animationManager;
	Ref<InputManager>				m_inputManager;
	//Ref<AudioManager>				m_audioManager;
	Ref<ShaderManager> m_shaderManager;
	Ref<GraphicsManager>			m_graphicsManager;
	Ref<MeshManager>				m_meshManager;
	Ref<RenderingManager>			m_renderingManager;
	//Ref<EffectManager>				m_effectManager;
	Ref<AssetManager>						m_assetManager;
	Ref<UIManager>					m_uiManager;

	Ref<UIFrameWindow> m_mainWindow;

	bool m_exitRequested;
};

} // namespace detail
} // namespace ln

