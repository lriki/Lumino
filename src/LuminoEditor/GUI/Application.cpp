﻿
#include "../../LuminoEngine/src/Engine/EngineManager.hpp"
#include "../../LuminoEngine/src/Engine/EngineDomain.hpp"
#include "../../LuminoEngine/src/Platform/PlatformManager.hpp"
#include "../../LuminoEngine/src/UI/UIManager.hpp"
#include "MainWindow.hpp"
#include "Application.hpp"

Ref<ln::UICommand> EditorApplication::NewCommand;
Ref<ln::UICommand> EditorApplication::OpenCommand;

static EditorApplication* s_app = nullptr;

EditorApplication* EditorApplication::instance()
{
    assert(s_app);
    return s_app;
}

EditorApplication::EditorApplication()
{
    assert(!s_app);
    s_app = this;

    NewCommand = ln::makeObject<ln::UICommand>();
    OpenCommand = ln::makeObject<ln::UICommand>();
}

EditorApplication::~EditorApplication()
{
    s_app = nullptr;
}

ln::Result EditorApplication::init()
{
	ln::EngineSettings::setMainWindowSize(1000, 600);
	ln::EngineSettings::setMainBackBufferSize(1000, 600);
    ln::detail::EngineDomain::engineManager()->settings().defaultObjectsCreation = false;
    ln::detail::EngineDomain::engineManager()->init();
    onInit();
    return true;
}

void EditorApplication::dispose()
{
    ln::detail::EngineDomain::release();
}

void EditorApplication::run()
{
    while (!ln::detail::EngineDomain::engineManager()->isExitRequested())
    {
        ln::detail::EngineDomain::engineManager()->platformManager()->windowManager()->processSystemEventQueue(ln::detail::EventProcessingMode::Polling);
        ln::detail::EngineDomain::engineManager()->uiManager()->dispatchPostedEvents();
        ln::Thread::sleep(1);
    }
}

ln::UIMainWindow* EditorApplication::mainWindow() const
{
    return ln::detail::EngineDomain::engineManager()->mainWindow();
}

void EditorApplication::setMainWindow(ln::UIMainWindow* window)
{
	return ln::detail::EngineDomain::engineManager()->setMainWindow(window);
}

void EditorApplication::onInit()
{
	setMainWindow(ln::makeObject<MainWindow>());
}