
#include "Internal.hpp"
#include <LuminoEngine/Engine/Application.hpp>
#include <LuminoEngine/Platform/CocoaPlatformInterface.hpp>

extern "C" ::ln::Application* LuminoCreateApplicationInstance();

namespace ln {

static ln::Application* g_app = nullptr;

int CocoaPlatformInterface::Main()
{
	g_app = ::LuminoCreateApplicationInstance();

	ln::detail::ApplicationHelper::initialize(g_app);
	ln::detail::ApplicationHelper::run(g_app);
	ln::detail::ApplicationHelper::finalize(g_app);
	ln::RefObjectHelper::release(g_app);
	g_app = nullptr;

	return 0;
}

} // namespace ln
