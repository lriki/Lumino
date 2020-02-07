
#include <EnvironmentSettings.hpp>
#include <Workspace.hpp>
#include <Project/Project.hpp>
#include "../src/Engine/EngineManager.hpp"
#include "NewCommand.hpp"
#include "FxcCommand.hpp"
#include "BuildCommand.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

static int commnad_localInitialSetup(const char* packageDir_);

static int processCommands(int argc, char** argv);

int main(int argc, char** argv)
{
#if defined(LN_DEBUG) && defined(_WIN32)
	if (argc == 1)
	{
        ::SetCurrentDirectoryW(L"D:/LocalProj/LN/Test1");
		//::SetCurrentDirectoryW(L"C:/Proj/LN/PrivateProjects/Roguelike2");
        ln::GlobalLogger::setLevel(ln::LogLevel::Verbose);
    
		const char* debugArgv[] = {
			"<program>",
			
			//"new", "Test1",
            
			//"build",
			"build", "Web",
			
			//"init", "RinoTutorial", "-t", "SimpleDesktop",
			//"build", "-p", "Windows"

			//"<program>", "dev-install-tools",

			

			//"run", "Web", //"Windows",

			//"dev-openide", "vs",
			
			//"--local-initial-setup", "/Users/lriki/Proj/Lumino/ReleasePackage.macOS"

			//"restore",

            //"fxc", "Assets/LineWave.fx",
            //"fxc", "C:/Proj/GitHub/Lumino/src/LuminoEngine/test/Assets/Shader/FxcTest1.fx",
            //"fxc", "D:/Proj/Volkoff/Engine/Lumino/src/LuminoEngine/src/Rendering/Resource/ClusteredShadingDefault.hlsl",

            //"build", "assets",
			//"fxc", "D:/Proj/Volkoff/Engine/Lumino/src/LuminoEngine/test/Assets/Graphics/SimplePosColor.fx"
            //"fxc", "D:/Proj/Volkoff/Engine/Lumino/src/LuminoEngine/test/Assets/Graphics/SimpleConstantBuffer.fx"
			//"fxc", "D:/Proj/Volkoff/Engine/Lumino/src/LuminoEngine/src/Graphics/Resource/VulkanSampleDeviceContext_26_shader_depth.fx",
			//"fxc", "C:/Proj/LN/Lumino/src/LuminoEngine/src/Graphics/Resource/VulkanSampleDeviceContext_26_shader_depth.fx",
		};
		argc = sizeof(debugArgv) / sizeof(char*);
		argv = (char**)debugArgv;

	}
#endif

	int exitCode = 0;
	try
	{
		setlocale(LC_ALL, "");

		if (argc == 3 && strcmp(argv[1], "--local-initial-setup") == 0)
		{
			return commnad_localInitialSetup(argv[2]);
		}
	
        ln::GlobalLogger::addStdErrAdapter();

		ln::EngineContext::current()->engineManager()->initializeAssetManager();

		exitCode = processCommands(argc, argv);

	}
	catch (ln::Exception& e)
	{
		ln::String m = e.getMessage();
		std::cout << m << std::endl;
		exitCode = 1;
	}

	ln::EngineContext::current()->disposeEngineManager();
	ln::EngineContext::current()->disposeRuntimeManager();

	return exitCode;
}

// .profile
static int commnad_localInitialSetup(const char* packageDir_)
{
	ln::Path packageDir = packageDir_;
	ln::Path toolsDir = ln::Path(packageDir, u"tools");
	ln::List<ln::String> lines;

	ln::Path profile = ln::Path(ln::Environment::specialFolderPath(ln::SpecialFolder::Home), u".profile");

	bool added = false;
	if (ln::FileSystem::existsFile(profile)) {
		int beginLine = -1;
		int endLine = -1;
		ln::StreamReader r(profile);
		ln::String line;
		while (r.readLine(&line)) {
			if (line.indexOf(u"# [Lumino begin]") == 0)
				beginLine = lines.size();
			if (line.indexOf(u"# [Lumino end]") == 0)
				endLine = lines.size();
			lines.add(line);
		}

		if (beginLine >= 0 && endLine >= 0 && beginLine < endLine) {
			// already exists.

			for (int i = beginLine; i < endLine; i++) {
				if (lines[i].indexOf("export LUMINO_PATH") == 0) {
					lines[i] = ln::String::format(u"export LUMINO_PATH={0}", packageDir);
					CLI::info(u"LUMINO_PATH updating.");
				}
				if (lines[i].indexOf("export PATH") == 0) {
					lines[i] = ln::String::format(u"export PATH=$PATH:{0}", toolsDir);
					CLI::info(u"PATH updating.");
				}
			}

			added = true;
		}
	}

	if (!added) {
		lines.add(u"");
		lines.add(u"# [Lumino begin]");
		lines.add(ln::String::format(u"export LUMINO_PATH={0}", packageDir));
		lines.add(ln::String::format(u"export PATH=$PATH:{0}", toolsDir));
		lines.add(u"# [Lumino end]");
	}

	ln::StreamWriter w(profile);
	w.setNewLine(u"\n");	// for macOS (\r is invalid)
	for (auto& line : lines) {
		w.writeLine(line);
	}

	CLI::info(ln::String::format(u"Lumino environment variable added to {0}.", profile));
	return 0;
}

static int processCommands(int argc, char** argv)
{

	ln::CommandLineParser parser;
	parser.addHelpOption();

	//auto langOption = parser.addNamedValueOption(u"l", u"lang", u"language.", { u"cpp", u"rb" });


	//--------------------------------------------------------------------------------
	// new command
	auto newCommand = parser.addCommand(u"new", u"Create a new project.");
	auto newCommand_pathArg = newCommand->addPositionalArgument(u"path", u"Project directory path.", ln::CommandLinePositionalArgumentFlags::Optional);
	auto newCommand_nameOption = newCommand->addValueOption(u"n", u"name", u" Set the resulting project name, defaults to the directory name.");
	auto newCommand_templateOption = newCommand->addValueOption(u"t", u"template", u"Project template.");
	auto newCommand_engineOption = newCommand->addValueOption(u"e", u"engine", u"Engine source.");

	//--------------------------------------------------------------------------------
	// build command
	auto buildCommand = parser.addCommand(u"build", u"Build the project.");
	auto buildCommand_packageOption = buildCommand->addFlagOption(u"p", u"package", u"Build release package.");
	auto burildTargetArg = buildCommand->addPositionalArgument(u"target", u"Specify the target to build.", ln::CommandLinePositionalArgumentFlags::Optional);

	//--------------------------------------------------------------------------------
	// run command
	auto runCommand = parser.addCommand(u"run", u"Run the project.");
	auto runCommand_targetArg = runCommand->addPositionalArgument(u"target", u"Specify the target to run.", ln::CommandLinePositionalArgumentFlags::Optional);

	//--------------------------------------------------------------------------------
	// restore command
	auto restoreCommand = parser.addCommand(u"restore", u"Restore engines included in the project.");

	//--------------------------------------------------------------------------------
	// fxc command
	auto fxcCommand = parser.addCommand(u"fxc", u"Compile shader.");
	auto fxcCommand_inputArg = fxcCommand->addPositionalArgument(u"input", u"Input file.");
	auto fxcCommand_outputArg = fxcCommand->addPositionalArgument(u"output", u"Output file.", ln::CommandLinePositionalArgumentFlags::Optional);

	//--------------------------------------------------------------------------------
	// build-assets command
	//auto buildAssetsCommand = parser.addCommand(u"build-assets", u"Make assets archive.");


	//--------------------------------------------------------------------------------
	auto dev_installTools = parser.addCommand(u"dev-install-tools", u"internal.");


	auto dev_openide = parser.addCommand(u"dev-openide", u"internal.");
	auto dev_openide_targetArg = dev_openide->addPositionalArgument(u"target", u"target.");


	if (parser.process(argc, argv))
	{
		auto workspace = ln::makeObject<lna::Workspace>();
		auto projectFile = lna::Workspace::findProejctFile(ln::Environment::currentDirectory());

		//--------------------------------------------------------------------------------
		// new command
		if (parser.has(newCommand))
		{
			NewCommand cmd;
			if (newCommand_nameOption->hasValue()) {
				cmd.projectName = newCommand_nameOption->value();
			}
			if (newCommand_templateOption->hasValue()) {
				cmd.templateName = newCommand_templateOption->value();
			}
			if (newCommand_engineOption->hasValue()) {
				cmd.engineSource = newCommand_engineOption->value();
			}
			if (newCommand_pathArg->hasValue()) {
				cmd.projectDirectory = newCommand_pathArg->value();
			}
			return cmd.execute(workspace);
		}
		//--------------------------------------------------------------------------------
		// build command
		else if (parser.has(buildCommand))
		{
			ln::String target = workspace->buildEnvironment()->defaultTargetName();
			if (burildTargetArg->hasValue()) {
				target = burildTargetArg->value();
			}

			if (!workspace->openMainProject(projectFile)) {
				return 1;
			}

			BuildCommand cmd;
			cmd.package = buildCommand_packageOption->isSet();
			cmd.target = target;
			return cmd.execute(workspace, workspace->mainProject());
		}
		//--------------------------------------------------------------------------------
		// run command
		else if (parser.has(runCommand))
		{
			ln::String target = workspace->buildEnvironment()->defaultTargetName();
			if (runCommand_targetArg->hasValue()) {
				target = runCommand_targetArg->value();
			}

			if (!workspace->openMainProject(projectFile)) {
				return 1;
			}


			BuildCommand cmd;
			cmd.target = target;
			if (cmd.execute(workspace, workspace->mainProject()) != 0) {
				return 1;
			}

			if (!workspace->runProject(target)) {
				return 1;
			}
		}
		//--------------------------------------------------------------------------------
		// restore command
		else if (parser.has(restoreCommand))
		{
			if (!workspace->openMainProject(projectFile)) {
				return 1;
			}
			if (!workspace->restoreProject()) {
				return 1;
			}
		}
		//--------------------------------------------------------------------------------
		// fxc command
		else if (parser.has(fxcCommand)) {
			FxcCommand cmd;
			if (fxcCommand_outputArg->hasValue()) {
				cmd.outputFile = fxcCommand_outputArg->value();
			}
			return cmd.execute(fxcCommand_inputArg->value());
		}
		//--------------------------------------------------------------------------------
		// build-assets command
		//else if (parser.has(buildAssetsCommand)) {
		//    if (!workspace->openProject(ln::Environment::currentDirectory())) {
		//        return 1;
		//    }
		//    ArchiveCommand cmd;
		//    return cmd.execute(workspace->project());
		//}
		//--------------------------------------------------------------------------------
		else if (parser.has(dev_installTools))
		{
			if (!workspace->dev_installTools()) {
				return 1;
			}
		}
		else if (parser.has(dev_openide))
		{
			if (!workspace->openMainProject(projectFile)) {
				return 1;
			}
			workspace->dev_openIde(dev_openide_targetArg->value());
		}
		else
		{
			parser.printHelp();
		}
	}
	else {
		parser.printHelp();
	}

	return 1;
}

