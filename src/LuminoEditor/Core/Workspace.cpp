﻿
#include "EnvironmentSettings.hpp"
#include "Project/Project.hpp"
#include "Project/AssetDatabase.hpp"
#include "Project/PluginManager.hpp"
#include "Workspace.hpp"

namespace lna {

static Workspace* s_instance = nullptr;

Workspace* Workspace::instance()
{
    assert(s_instance);
    return s_instance;
}

//ln::Result Workspace::init()
//{
//    assert(!s_instance);
//    s_instance = new Workspace();
//    return true;
//}
//
//void Workspace::finalize()
//{
//    if (s_instance) {
//        delete s_instance;
//        s_instance = nullptr;
//    }
//}

Workspace::Workspace()
	: m_buildEnvironment(ln::makeRef<BuildEnvironment>())
{
    assert(!s_instance);
    s_instance = this;
#ifdef LN_DEBUG
	m_buildEnvironment->setupPathes(EnvironmentPathBase::Repository);
#else
	m_buildEnvironment->setupPathes(EnvironmentPathBase::EnvironmentVariable);
#endif
}

Workspace::~Workspace()
{
    s_instance = nullptr;
}

ln::Result Workspace::newMainProject(const ln::Path& projectDir, const ln::String& projectName)
{
	if (LN_REQUIRE(!m_mainProject)) return false;
    m_mainProject = ln::makeRef<Project>(this);
    if (!m_mainProject->newProject(projectDir, projectName, u"", u"NativeProject")) {
        m_mainProject = nullptr;
        return false;
    }
    postMainProjectLoaded();
    return true;
}

ln::Result Workspace::openMainProject(const ln::Path& filePath)
{
	if (LN_REQUIRE(!m_mainProject)) return false;
    m_mainProject = ln::makeRef<Project>(this);
    if (!m_mainProject->openProject2(filePath)) {
        m_mainProject = nullptr;
        return false;
    }
    postMainProjectLoaded();
    return true;
}

ln::Result Workspace::closeMainProject()
{
    m_mainProject = nullptr;
	return true;
}

ln::Result Workspace::runProject(const ln::String& target)
{
	// Windows
	if (ln::String::compare(target, u"Windows", ln::CaseSensitivity::CaseInsensitive) == 0)
	{
		auto exe = ln::FileSystem::getFile(ln::Path(m_mainProject->windowsProjectDir(), u"bin/Debug"), u"*.exe");
		ln::Process::execute(exe);
	}
	// Web
	else if (ln::String::compare(target, u"Web", ln::CaseSensitivity::CaseInsensitive) == 0)
	{
		auto buildDir = ln::Path::combine(m_mainProject->buildDir(), u"Web").canonicalize();

		ln::Process proc;
		proc.setProgram(m_buildEnvironment->python2());
		proc.setArguments({u"-m", u"SimpleHTTPServer", u"8000"});
		proc.setWorkingDirectory(buildDir);
		proc.setUseShellExecute(false);
		proc.start();

		{
			auto files = ln::FileSystem::getFiles(buildDir, u"*.html");
			if (files.isEmpty()) {
				CLI::error("Not found *.html file.");
				return false;
			}

			ln::Process proc2;
            proc2.setUseShellExecute(true);
			proc2.setProgram(u"http://localhost:8000/" + (*files.begin()).fileName());
			proc2.start();
		}

		proc.wait();
	}
	else
	{
		CLI::error(ln::String::format(u"{0} is invalid target.", target));
		return false;
	}

	return true;
}

ln::Result Workspace::restoreProject()
{
    m_mainProject->restore();
    return true;
}

ln::Result Workspace::dev_installTools() const
{
	m_buildEnvironment->prepareEmscriptenSdk();
    return true;
}

void Workspace::dev_openIde(const ln::String& target) const
{
#if defined(_WIN32)
	if (ln::String::compare(target, u"Android", ln::CaseSensitivity::CaseInsensitive) == 0)
	{
		HKEY hKey = NULL;
		LONG lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\Android Studio",
			NULL,
			KEY_READ | KEY_WOW64_64KEY,	// https://stackoverflow.com/questions/252297/why-is-regopenkeyex-returning-error-code-2-on-vista-64bit
			&hKey);
		if (lRet != ERROR_SUCCESS) {
			LN_LOG_ERROR << u"Android Studio not installed.";
			return;
		}

		DWORD type, size;
		WCHAR path[MAX_PATH];
		RegQueryValueExW(hKey, L"Path", NULL, &type, (LPBYTE)path, &size);

		ln::Environment::setEnvironmentVariable(u"LUMINO", buildEnvironment()->luminoPackageRootDir());

		ln::Process proc;
		proc.setProgram(ln::Path::combine(ln::String::fromCString(path), u"bin", u"studio"));
		proc.setArguments({ m_mainProject->androidProjectDir() });
		proc.start();
	}
	else
	{
		auto files = ln::FileSystem::getFiles(ln::Environment::currentDirectory(), u"*.sln");
		if (files.isEmpty()) {
			CLI::error("Not found *.sln file.");
			return;
		}

		ln::Environment::setEnvironmentVariable(u"LUMINO", buildEnvironment()->luminoPackageRootDir());

		ln::Process proc;
		proc.setProgram(*files.begin());
		proc.start();
	}
#elif defined(__APPLE__)
	ln::Process proc;
	proc.setProgram(u"/usr/bin/open");
	proc.setArguments({ u"/Applications/Xcode.app/", ln::Path(m_project->macOSProjectDir(), u"LuminoApp.macOS.xcodeproj") });
	proc.start();
#else
	LN_NOTIMPLEMENTED();	// TODO: putenv は書き込み可能なポインタを渡さないとならないみたい？
#endif
}

ln::Path Workspace::findProejctFile(const ln::Path& dir)
{
    auto files = ln::FileSystem::getFiles(dir, u"*" + Project::ProjectFileExt);
    if (!files.isEmpty()) {
    	return *files.begin();
    }
    else {
        return ln::Path();
    }
}

void Workspace::postMainProjectLoaded()
{
    m_mainAssetDatabase = ln::makeRef<AssetDatabase>();
    if (!m_mainAssetDatabase->init(m_mainProject)) {
        return;
    }

    m_mainPluginManager = ln::makeRef<PluginManager>();
    if (!m_mainPluginManager->init(m_mainProject)) {
        return;
    }

    m_mainPluginManager->reloadPlugins();
}

} // namespace lna
