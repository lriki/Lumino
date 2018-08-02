﻿using System;
using System.Collections.Generic;
using System.IO;

namespace LuminoBuild.Tasks
{
    class BuildEngineEmscripten : BuildTask
    {
        public override string CommandName { get { return "BuildEngineEmscripten"; } }

        public override string Description { get { return "Build Emscripten"; } }

        public override void Build(Builder builder)
        {
            var buildArchDir = "Emscripten";

            var buildDir = Path.Combine(builder.LuminoBuildDir, buildArchDir);
            var installDir = Path.Combine(builder.LuminoBuildDir, "CMakeInstallTemp", buildArchDir);
            var cmakeSourceDir = builder.LuminoRootDir;

            Directory.CreateDirectory(buildDir);

            var script = Path.Combine(buildDir, "build.bat");
            using (var f = new StreamWriter(script))
            {
                f.WriteLine($"cd \"{BuildEnvironment.EmsdkDir}\"");
                f.WriteLine($"call emsdk_env.bat");
                f.WriteLine($"cd \"{Utils.ToWin32Path(buildDir)}\"");
                f.WriteLine($"call emcmake cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX={installDir} -G \"MinGW Makefiles\" {cmakeSourceDir}");
                f.WriteLine($"call cmake --build .");
                f.WriteLine($"call cmake --build . --target install");
            }

            Utils.CallProcessShell(script); // bat の中でエラーが発生すれば、例外に乗って出てくる



            //string emRootDir = BuildEnvironment.EmscriptenDir;
            //string emInstallDir = BuildEnvironment.EmscriptenDir;
            //string bundlePythonDir = Path.Combine(emInstallDir, "python", "2.7.5.3_64bit");

            //string cmakeOutputDir = Path.Combine(builder.LuminoBuildDir, "CMakeInstallTemp", "Emscripten");

            //string path = Environment.GetEnvironmentVariable("PATH");
            //path = bundlePythonDir + ";" + path;

            //var environmentVariables = new Dictionary<string, string>()
            //{
            //    { "PATH", path }
            //};

            //string buildDir = Path.Combine(builder.LuminoBuildDir, "Emscripten");
            //Directory.CreateDirectory(buildDir);
            //Directory.SetCurrentDirectory(buildDir);

            //Utils.CallProcess(BuildEnvironment.emcmake, $"cmake {builder.LuminoRootDir} -DCMAKE_INSTALL_PREFIX={cmakeOutputDir} -G \"MinGW Makefiles\"", environmentVariables);
            //Utils.CallProcess("cmake", $"--build {buildDir}", environmentVariables);
            //Utils.CallProcess("cmake", $"--build {buildDir} --target install", environmentVariables);
        }
    }
}
