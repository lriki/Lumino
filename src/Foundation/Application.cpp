﻿
#include "../Internal.h"
#include <Lumino/Engine.h>
#include <Lumino/Foundation/Application.h>
#include "../EngineManager.h"

LN_NAMESPACE_BEGIN

//=============================================================================
// Application
//=============================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Application::Application()
	: m_engineManager(nullptr)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Application::~Application()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Application::Initialize(EngineManager* engineManager)
{
	LN_CHECK_ARGS_RETURN(engineManager != nullptr);
	m_engineManager = engineManager;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PlatformWindow* Application::GetNativeMainWindow()
{
	return m_engineManager->GetPlatformManager()->GetMainWindow();
}

//=============================================================================
// GameApplication
//=============================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GameApplication::GameApplication()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GameApplication::~GameApplication()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GameApplication::OnConfigure(EngineSettings* settings)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GameApplication::Run()
{
	try
	{
		EngineSettings data;
		data.graphicsAPI = GraphicsAPI::DirectX9;
		data.renderingType = RenderingType::Immediate;

		OnConfigure(&data);

		Engine::Initialize(data);

		do
		{

		} while (Engine::UpdateFrame());
	}
	catch (...)
	{
		Engine::Finalize();
		throw;
	}
	Engine::Finalize();
}

LN_NAMESPACE_END
