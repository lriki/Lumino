
#pragma once

#include <Lumino/Base/RefObject.h>
#include <Lumino/Base/Size.h>
#include <Lumino/Base/String.h>
#include <Lumino/Base/Array.h>
#include <Lumino/Threading/Thread.h>
#include <Lumino/Platform/PlatformManager.h>
#include <Lumino/Physics/PhysicsManager.h>
#include <Lumino/Graphics/GraphicsManager.h>
#include <Lumino/Documents/DocumentsManager.h>
#include <Lumino/GUI/GUIManager.h>
#include <Lumino/Game/FpsController.h>
#include <Lumino/ApplicationSettings.h>

namespace Lumino
{
namespace Audio { class AudioManager; }
namespace Graphics { class ProfilerRenderer; }
class SceneGraphManager;

class ApplicationImpl
	: public RefObject
	, public Platform::IEventListener
{
public:
	static ApplicationImpl*	Instance;

public:
	

public:

	/**
		@brief		Application のインスタンスを作成し、アプリケーションを初期化します。
	*/
	static ApplicationImpl* Create(const ApplicationSettings& configData);

public:

	/**
		@brief		
		@return		アプリケーションの終了が要求されている場合は false を返します。
	*/
	bool UpdateFrame();

	/**
		@brief		
	*/
	void Render();

	/**
		@brief		遅延をリセットします。
		@details	リソースのロード等で時間がかかり長い時間更新処理が行われなかった場合、
					UpdateFrame() は本来あるべき時間に追いつこうとしてしばらくの間ノーウェイトでフレーム更新が行われます。
					その間はアプリケーションが非常に高速に動作しているように見えてしまします。
					これを回避するため、時間のかかる処理の直後でこの関数を呼ぶことで、FPS 制御に遅延が発生していないことを伝えます。
	*/
	void ResetFrameDelay();

	/**
		@brief		アプリケーションの終了が要求されているかを確認します。
	*/
	bool IsEndRequested() const { return m_endRequested; }


	Platform::PlatformManager* GetPlatformManager() const { return m_platformManager; }
	Audio::AudioManager* GetAudioManager() const { return m_audioManager; }
	Physics::PhysicsManager* GetPhysicsManager() const { return m_physicsManager; }
	Graphics::GraphicsManager* GetGraphicsManager() const { return m_graphicsManager; }
	Documents::DocumentsManager* GetDocumentsManager() const { return m_documentsManager; }
	GUIManager* GetGUIManager() const { return m_guiManager; }
	SceneGraphManager* GetSceneGraphManager() const { return m_sceneGraphManager; }

protected:
	ApplicationImpl(const ApplicationSettings& configData);
	virtual ~ApplicationImpl();

public:
	void Initialize();
	void InitialzePlatformManager();
	void InitialzeAudioManager();
	void InitialzePhysicsManager();
	void InitialzeGraphicsManager();
	void InitialzeDocumentsManager();
	void InitialzeGUIManager();
	void InitialzeSceneGraphManager();
	virtual bool OnEvent(const Platform::EventArgs& e);

private:
	//class NativeWindowEventListener;
	ApplicationSettings				m_configData;
	FpsController						m_fpsController;
	RefPtr<Platform::PlatformManager>	m_platformManager;
	Audio::AudioManager*				m_audioManager;
	RefPtr<Physics::PhysicsManager>		m_physicsManager;
	RefPtr<Graphics::GraphicsManager>	m_graphicsManager;
	RefPtr<Documents::DocumentsManager>	m_documentsManager;
	RefPtr<GUIManager>				m_guiManager;
	SceneGraphManager*					m_sceneGraphManager;

	Graphics::ProfilerRenderer*			m_profilerRenderer;
	
	//NativeWindowEventListener*			m_nativeWindowEventListener;
	bool								m_endRequested;
};

} // namespace Lumino
