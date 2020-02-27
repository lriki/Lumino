﻿using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;
using LuminoBuild;

namespace LuminoBuild.Tasks
{
    // Debug and Release build:
    //      dotnet run -- BuildEngine_MSVC MSVC2017-x64-MD
    //
    // Debug only:
    //      dotnet run -- BuildEngine_MSVC MSVC2017-x64-MD Debug
    //
    // Release only:
    //      dotnet run -- BuildEngine_MSVC MSVC2017-x64-MD Release
    //
    class BuildEngine_MSVC : BuildTask
    {
        public override string CommandName => "BuildEngine_MSVC";

        //public override List<string> Dependencies => new List<string>() { "BuildExternalProjects" };

        public class MSVCTargetInfo
        {
            public string Generator;
            public string Arch;
            public string StaticRuntime;
        }

        public static Dictionary<string, MSVCTargetInfo> TargetInfoMap = new Dictionary<string, MSVCTargetInfo>()
        {
            //{ "MSVC2017-x86-MD", new MSVCTargetInfo { Generator = "Visual Studio 15", Arch="Win32", StaticRuntime = "OFF" } },
            //{ "MSVC2017-x86-MT", new MSVCTargetInfo { Generator = "Visual Studio 15", Arch="Win32", StaticRuntime = "ON" } },
            //{ "MSVC2017-x64-MD", new MSVCTargetInfo { Generator = "Visual Studio 15 Win64", Arch="x64", StaticRuntime = "OFF" } },
            { "MSVC2017-x64-MT", new MSVCTargetInfo { Generator = "Visual Studio 15 Win64", Arch="", StaticRuntime = "ON" } },
            { "MSVC2019-x64-MT", new MSVCTargetInfo { Generator = "Visual Studio 16 2019", Arch="x64", StaticRuntime = "ON" } },
        };

        public override void Build(Builder builder)
        {
            if (string.IsNullOrEmpty(BuildEnvironment.Target))
            {
                foreach (var i in TargetInfoMap)
                {
                    BuildTarget(builder, i.Key, i.Value);
                }
            }
            else
            {
                // Run mainly from CI
                var targetInfo = TargetInfoMap[BuildEnvironment.Target];
                BuildTarget(builder, BuildEnvironment.Target, targetInfo);
            }
        }

        private void BuildTarget(Builder builder, string targetName, MSVCTargetInfo targetInfo)
        {
            var fileMoving = false;

            var targetBuildDir = Path.Combine(builder.LuminoBuildDir, targetName);
            var installDir = Path.Combine(builder.LuminoRootDir, "build", targetName, BuildEnvironment.EngineInstallDirName);

            Directory.CreateDirectory(targetBuildDir);
            Directory.SetCurrentDirectory(targetBuildDir);

            // Configuration
            {

                var additional = "";
                if (!string.IsNullOrEmpty(targetInfo.Arch))
                {
                    additional += "-A " + targetInfo.Arch;
                }

                var args = new string[]
                {
                    $"-G\"{targetInfo.Generator}\"",
                    $"-DCMAKE_INSTALL_PREFIX=\"{installDir}\"",
                    $"-DCMAKE_DEBUG_POSTFIX=d",
                    $"-DLN_MSVC_STATIC_RUNTIME={targetInfo.StaticRuntime}",
                    $"-DLN_BUILD_TESTS=ON",
                    $"-DLN_BUILD_EDITOR=ON",
                    $"-DLN_BUILD_SHARED_LIBRARY=ON",
                    $"-DLN_BUILD_EMBEDDED_SHADER_TRANSCOMPILER=ON",
                    $"-DLN_TARGET_ARCH:STRING={targetName}",
                    additional,
                    $" ../..",
                };
                Utils.CallProcess("cmake", string.Join(' ', args));

                // ポストイベントからファイルコピーが行われるため、先にフォルダを作っておく
                Directory.CreateDirectory(Path.Combine(builder.LuminoLibDir, targetName));
            }

            // Build
            {
                if (string.IsNullOrEmpty(BuildEnvironment.Configuration) || BuildEnvironment.Configuration == "Debug")
                {
                    Utils.CallProcess("cmake", $"--build . --config Debug");
                    Utils.CallProcess("ctest", $"-C Debug --output-on-failure --verbose");
                    Utils.CallProcess("cmake", $"--build . --config Debug --target INSTALL");
                }

                if (string.IsNullOrEmpty(BuildEnvironment.Configuration) || BuildEnvironment.Configuration == "Release")
                {
                    Utils.CallProcess("cmake", $"--build . --config Release");
                    Utils.CallProcess("ctest", $"-C Release --output-on-failure --verbose");
                    Utils.CallProcess("cmake", $"--build . --config Release --target INSTALL");
                }
            }

            // Copy external libs to EngineInstall dir
            {
                var externalInstallDir = Path.Combine(builder.LuminoBuildDir, targetName, "ExternalInstall");
                var installLibDir = Path.Combine(installDir, "lib");

                foreach (var dir in Directory.GetDirectories(externalInstallDir))
                {
                    var libDir = Path.Combine(dir, "lib");
                    if (Directory.Exists(libDir))
                    {
                        foreach (var file in Directory.GetFiles(libDir, "*.lib", SearchOption.TopDirectoryOnly))
                        {
                            if (fileMoving)
                                File.Move(file, Path.Combine(installLibDir, Path.GetFileName(file)));
                            else
                                File.Copy(file, Path.Combine(installLibDir, Path.GetFileName(file)), true);
                        }
                    }
                }
            }

            // Copy .pdb
            {
                // CMake では static library の PDB 出力先をコントロールできない。https://cmake.org/cmake/help/v3.1/prop_tgt/PDB_OUTPUT_DIRECTORY.html
                // そのためビルドスクリプト側でコントロールする。
                // 以下、パスに "Debug" を含むもののうち、lib と同じ名前の pdb ファイルをコピーする。

                var installLibDir = Path.Combine(installDir, "lib");
                var libfiles = Directory.GetFiles(installLibDir, "*.lib", SearchOption.TopDirectoryOnly);
                var libnames = new HashSet<string>(libfiles.Select(x => Path.GetFileNameWithoutExtension(x)));
                var files1 = Directory.GetFiles(targetBuildDir, "*.pdb", SearchOption.AllDirectories);
                foreach (var file in files1)
                {
                    if (file.Contains("Debug") && libnames.Contains(Path.GetFileNameWithoutExtension(file)))
                    {
                        // FIXME: CI サーバのストレージ不足対策
                        if (fileMoving)
                            File.Move(file, Path.Combine(installLibDir, Path.GetFileName(file)));
                        else
                            File.Copy(file, Path.Combine(installLibDir, Path.GetFileName(file)), true);
                    }
                }
            }
        }
    }
}
