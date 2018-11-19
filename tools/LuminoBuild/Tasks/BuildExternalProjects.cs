﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace LuminoBuild.Tasks
{
    /*
     * build/MSVC2017-x64-MT/ の下など、各アーキテクチャフォルダの中に
     * - ExternalBuild
     * - ExternalInstall    → ライブラリのインストールフォルダ 
     * を作る。
     */
    class BuildExternalProjects : BuildTask
    {
        public override string CommandName { get { return "BuildExternalProjects"; } }

        public override string Description { get { return "BuildExternalProjects"; } }


        // (システム標準の cmake を使う系)
        private void BuildProject(Builder builder, string projectDirName, string buildType, string externalSourceDir, string buildArch, string generator, string additionalOptions = "")
        {
            var projectName = Path.GetFileName(projectDirName); // zlib/contrib/minizip のような場合に minizip だけ取り出す
            var targetName = buildArch + "-" + buildType;
            var buildDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, targetName, "ExternalBuild", projectName));
            var installDir = Path.Combine(builder.LuminoBuildDir, targetName, "ExternalInstall", projectName);
            var cmakeSourceDir = Path.Combine(externalSourceDir, projectDirName);
            var ov = Path.Combine(builder.LuminoRootDir, "src", "CFlagOverrides.cmake");

            Logger.WriteLine($"BuildProject ({projectDirName}) buildDir:{buildDir}");

            Directory.CreateDirectory(buildDir);
            Directory.SetCurrentDirectory(buildDir);
            
            var args = new string[]
            {
                $"-DCMAKE_INSTALL_PREFIX={installDir}",
                $"-DCMAKE_USER_MAKE_RULES_OVERRIDE={ov}",
                $"{additionalOptions}",
                $"-G \"{generator}\"",
                $"{cmakeSourceDir}",
            };
            Utils.CallProcess("cmake", string.Join(' ', args));
            Utils.CallProcess("cmake", $"--build . --config {buildType}");
            Utils.CallProcess("cmake", $"--build . --config {buildType} --target install");
            //Utils.CallProcess("cmake", "--build . --config Release");
            //Utils.CallProcess("cmake", "--build . --config Release --target install");

            /*
                MSVC と Xcode は Debug,Release の 2 つの構成をもつプロジェクトが出力される。
            */
        }

        private void BuildProjectEm(Builder builder, string projectDirName, string externalSourceDir, string buildArchDir, string additionalOptions = "")
        {
            var projectName = Path.GetFileName(projectDirName);
            var buildDir = Path.Combine(builder.LuminoBuildDir, buildArchDir, "ExternalBuild", projectName);
            var installDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, buildArchDir, "ExternalInstall", projectName));
            var cmakeSourceDir = Utils.ToUnixPath(Path.Combine(externalSourceDir, projectDirName));

            Logger.WriteLine($"BuildProjectEm ({projectDirName}) buildDir:{buildDir}");

            Directory.CreateDirectory(buildDir);

            var script = Path.Combine(buildDir, "build.bat");
            using (var f = new StreamWriter(script))
            {
                f.WriteLine($"cd /d \"{BuildEnvironment.EmsdkDir}\"");
                f.WriteLine($"call emsdk activate {BuildEnvironment.emsdkVer}");
                f.WriteLine($"call emsdk_env.bat");
                f.WriteLine($"cd /d \"{Utils.ToWin32Path(buildDir)}\"");
                f.WriteLine($"call emcmake cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX={installDir} {additionalOptions} -G \"MinGW Makefiles\" {cmakeSourceDir}");
                f.WriteLine($"call cmake --build .");
                f.WriteLine($"call cmake --build . --target install");
            }

            Utils.CallProcess(script); // bat の中でエラーが発生すれば、例外に乗って出てくる
        }

        private void BuildProjectAndroid(Builder builder, string projectDirName, string externalSourceDir, string abi, string buildType, string additionalOptions = "")
        {
            var projectName = Path.GetFileName(projectDirName);
            string cmakeHomeDir = Path.Combine(externalSourceDir, projectDirName);//builder.LuminoRootDir;
            string platform = BuildEnvironment.AndroidTargetPlatform;
            
            var targetName = $"Android-{abi}-{buildType}";
            var cmakeBuildDir = Path.Combine(builder.LuminoBuildDir, targetName, "ExternalBuild", projectName);
            var cmakeInstallDir = Path.Combine(builder.LuminoBuildDir, targetName, "ExternalInstall", projectName);

            var args = new string[]
            {
                $"-H{cmakeHomeDir}",
                $"-B{cmakeBuildDir}",
                $"-DLN_TARGET_ARCH_NAME={targetName}",
                $"-DCMAKE_INSTALL_PREFIX={cmakeInstallDir}",
                $"-DANDROID_ABI={abi}",
                $"-DANDROID_PLATFORM={platform}",
                $"-DCMAKE_BUILD_TYPE={buildType}",
                $"-DANDROID_NDK={BuildEnvironment.AndroidNdkRootDir}",
                $"-DCMAKE_CXX_FLAGS=-std=c++14",
                $"-DANDROID_STL=c++_shared",
                $"-DCMAKE_TOOLCHAIN_FILE={BuildEnvironment.AndroidCMakeToolchain}",
                $"-DCMAKE_MAKE_PROGRAM={BuildEnvironment.AndroidSdkNinja}",
                additionalOptions,
                $"-G\"Android Gradle - Ninja\"",
            };

            Utils.CallProcess(BuildEnvironment.AndroidSdkCMake, string.Join(' ', args));
            Utils.CallProcess(BuildEnvironment.AndroidSdkCMake, "--build " + cmakeBuildDir);
            Utils.CallProcess(BuildEnvironment.AndroidSdkCMake, "--build " + cmakeBuildDir + " --target install");
        }

        public override void Build(Builder builder)
        {
            var reposDir = Path.Combine(builder.LuminoBuildDir, "ExternalSource");
            Directory.CreateDirectory(reposDir);
            Directory.SetCurrentDirectory(reposDir);

            if (!Directory.Exists("ios-cmake"))
            {
                Utils.CallProcess("git", "clone --progress --depth 1 -b 2.0.0 https://github.com/leetal/ios-cmake.git ios-cmake");
            }
            if (!Directory.Exists("zlib"))
            {
                Utils.CallProcess("git", "clone --progress --depth 1 -b v1.2.11 https://github.com/madler/zlib.git zlib");
                Utils.CopyFile(Path.Combine(builder.LuminoExternalDir, "zlib", "CMakeLists.txt"), "zlib");
                Utils.CopyFile(Path.Combine(builder.LuminoExternalDir, "minizip", "CMakeLists.txt"), "zlib/contrib/minizip");
            }
            if (!Directory.Exists("libpng"))
            {
                //Utils.CallProcess("git", "clone --progress --depth 1 -b v1.6.9 git://git.code.sf.net/p/libpng/code libpng");
                Utils.CallProcess("git", "clone --progress --depth 1 -b libpng17 https://github.com/glennrp/libpng.git libpng");

#if false
                var zip = Path.Combine(reposDir, "lpng1635.zip");
                Utils.DownloadFile("https://download.sourceforge.net/libpng/lpng1635.zip", zip);

                var dir = Path.Combine(reposDir, "lpng1635");
                Utils.ExtractZipFile(zip, dir);
                Directory.Move(Path.Combine(dir, "lpng1635"), Path.Combine(reposDir, "libpng"));
                //Directory.Move(dir, Path.Combine(reposDir, "libpng"));

                //Utils.CallProcess("git", "clone --progress --depth 1 -b libpng17 https://github.com/glennrp/libpng.git libpng");
#endif
                Utils.CopyFile(Path.Combine(builder.LuminoExternalDir, "libpng", "CMakeLists.txt"), "libpng");
            }
            if (!Directory.Exists("glslang"))
            {
                Utils.CallProcess("git", "clone --progress --depth 1 -b 7.9.2888 https://github.com/KhronosGroup/glslang.git glslang");
            }
            if (!Directory.Exists("SPIRV-Cross"))
            {
                Utils.CallProcess("git", "clone --progress https://github.com/KhronosGroup/SPIRV-Cross.git SPIRV-Cross");
                Directory.SetCurrentDirectory("SPIRV-Cross");
                Utils.CallProcess("git", "checkout be7425ef70231ab82930331959ab487d605d0482");
                Directory.SetCurrentDirectory(reposDir);
            }
            if (!Directory.Exists("glfw"))
            {
                // TODO: #glfw 816 の対策が現時点の最新 3.2.1 には入っていないので、開発中の master を取ってくる
                // 3.3 リリース後、そのタグで clone するようにしておく。
                if (Utils.IsMac)
                {
                    Utils.CallProcess("git", "clone --progress https://github.com/glfw/glfw.git glfw");
                    Directory.SetCurrentDirectory("glfw");
                    Utils.CallProcess("git", "checkout 5afcd0981bf2fe9b9550f24ba298857aac6c35c2");
                    Directory.SetCurrentDirectory(reposDir);
                }
                else
                {
                    Utils.CallProcess("git", "clone --progress --depth 1 -b 3.2.1 https://github.com/glfw/glfw.git glfw");
                }
            }
            if (!Directory.Exists("glad"))
            {
                Utils.CopyDirectory(Path.Combine(builder.LuminoExternalDir, "glad"), "glad");
                //Utils.CallProcess("git", "clone --progress --depth 1 -b v0.1.26 https://github.com/Dav1dde/glad.git glad");
            }
            if (!Directory.Exists("openal-soft"))
            {
                // https://github.com/kcat/openal-soft/issues/183 の問題の修正後、まだタグが降られていない。そのため latest を取得
                Utils.CallProcess("git", "clone --progress https://github.com/kcat/openal-soft.git openal-soft");
                Directory.SetCurrentDirectory("openal-soft");
                Utils.CallProcess("git", "checkout 7d76cbddd6fbdb52eaa917845435b95ae89efced");
                Directory.SetCurrentDirectory(reposDir);
            }
            if (!Directory.Exists("SDL2"))
            {
                using (var wc = new System.Net.WebClient())
                {
                    var zip = Path.Combine(reposDir, "SDL2-2.0.8.zip");
                    wc.DownloadFile("https://www.libsdl.org/release/SDL2-2.0.8.zip", zip);
                    Utils.ExtractZipFile(zip, Path.Combine(reposDir, "SDL2-2.0.8"));
                    Directory.Move(Path.Combine(reposDir, "SDL2-2.0.8", "SDL2-2.0.8"), Path.Combine(reposDir, "SDL2"));
                }
            }
            if (!Directory.Exists("freetype2"))
            {
                // freetype2 の CMakeList.txt は iOS ツールチェインを独自で持っているが、
                // SIMULATOR64 のサポートは 2018/11/19 時点ではタグが降られていないため、現時点の最新を使う。
                Utils.CallProcess("git", "clone --progress git://git.sv.nongnu.org/freetype/freetype2.git freetype2");
                Directory.SetCurrentDirectory("freetype2");
                Utils.CallProcess("git", "checkout c13635ee4bf34e621816cd09d7f2baf918e20af8");
                Directory.SetCurrentDirectory(reposDir);
            }
            if (!Directory.Exists("ogg"))
            {
                Utils.CallProcess("git", "clone --progress --depth 1 -b v1.3.3 https://github.com/xiph/ogg.git ogg");
            }
            if (!Directory.Exists("vorbis"))
            {
                Utils.CallProcess("git", "clone --progress --depth 1 -b v1.3.6 https://github.com/xiph/vorbis.git vorbis");
            }


            if (Utils.IsWin32)
            {
                // Android
                if (BuildEnvironment.AndroidStudioFound)
                {
                    foreach (var target in BuildEngine_AndroidJNI.Targets)
                    {
                        var zlibInstallDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, $"Android-{target.ABI}-{target.BuildType}", "ExternalInstall", "zlib"));
                        var oggInstallDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, $"Android-{target.ABI}-{target.BuildType}", "ExternalInstall", "ogg"));

                        BuildProjectAndroid(builder, "zlib", reposDir, target.ABI, target.BuildType);
                        BuildProjectAndroid(builder, "zlib/contrib/minizip", reposDir, target.ABI, target.BuildType, $"-DZLIB_INCLUDE_DIR={zlibInstallDir}/include");
                        BuildProjectAndroid(builder, "libpng", reposDir, target.ABI, target.BuildType, $"-DZLIB_INCLUDE_DIR={zlibInstallDir}/include");
                        BuildProjectAndroid(builder, "freetype2", reposDir, target.ABI, target.BuildType);
                        BuildProjectAndroid(builder, "ogg", reposDir, target.ABI, target.BuildType);
                        BuildProjectAndroid(builder, "vorbis", reposDir, target.ABI, target.BuildType, $"-DOGG_ROOT={oggInstallDir} -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH");
                    }
                }

                // Emscripten
                {
                    var externalInstallDir = Path.Combine(builder.LuminoBuildDir, "Emscripten", "ExternalInstall");
                    var zlibInstallDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, "Emscripten", "ExternalInstall", "zlib"));
                    var oggInstallDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, "Emscripten", "ExternalInstall", "ogg"));

                    BuildProjectEm(builder, "zlib", reposDir, "Emscripten");
                    BuildProjectEm(builder, "zlib/contrib/minizip", reposDir, "Emscripten", $"-DZLIB_INCLUDE_DIR={zlibInstallDir}/include");
                    BuildProjectEm(builder, "libpng", reposDir, "Emscripten", $"-DZLIB_INCLUDE_DIR={zlibInstallDir}/include");
                    BuildProjectEm(builder, "glad", reposDir, "Emscripten", "-DGLAD_INSTALL=ON");
                    BuildProjectEm(builder, "freetype2", reposDir, "Emscripten");
                    BuildProjectEm(builder, "ogg", reposDir, "Emscripten");
                    BuildProjectEm(builder, "vorbis", reposDir, "Emscripten", $"-DOGG_ROOT={oggInstallDir} -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH");
                }

                // Visual C++
                foreach (var target in MakeVSProjects.Targets)
                {
                    var zlibInstallDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, $"{target.DirName}-{target.BuildType}", "ExternalInstall", "zlib"));
                    var oggInstallDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, $"{target.DirName}-{target.BuildType}", "ExternalInstall", "ogg"));

                    BuildProject(builder, "zlib", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime}");
                    BuildProject(builder, "zlib/contrib/minizip", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime} -DZLIB_INCLUDE_DIR={zlibInstallDir}/include");
                    BuildProject(builder, "libpng", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime} -DZLIB_INCLUDE_DIR={zlibInstallDir}/include");
                    BuildProject(builder, "glslang", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime}");
                    BuildProject(builder, "SPIRV-Cross", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime}");
                    BuildProject(builder, "glfw", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime} -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=ON");
                    BuildProject(builder, "glad", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime} -DGLAD_INSTALL=ON");
                    BuildProject(builder, "openal-soft", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime}");
                    BuildProject(builder, "SDL2", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DSDL_SHARED=OFF -DSDL_STATIC=ON -DSSE=OFF -DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime}");
                    BuildProject(builder, "freetype2", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime}");
                    BuildProject(builder, "ogg", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime}");
                    BuildProject(builder, "vorbis", target.BuildType, reposDir, target.DirName, target.VSTarget, $"-DLN_MSVC_STATIC_RUNTIME={target.MSVCStaticRuntime} -DOGG_ROOT={oggInstallDir}");
                }
            }
            else
            {
                // iOS
                {
                    var targetInfos = new []
                    {
                        new { Config = "Debug", Platform = "OS" },
                        new { Config = "Release", Platform = "OS" },
                        new { Config = "Debug", Platform = "SIMULATOR64" },
                        new { Config = "Release", Platform = "SIMULATOR64" },
                    };
                    var iOSToolchainFile = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, "ExternalSource", "ios-cmake", "ios.toolchain.cmake "));
                    
                    foreach (var t in targetInfos)
                    {
                        var dirName = $"iOS-{t.Platform}-{t.Config}";
                        var args = $"-DCMAKE_TOOLCHAIN_FILE=\"{iOSToolchainFile}\" -DIOS_PLATFORM={t.Platform}";
                        var generator = "Xcode";
                        
                        var oggInstallDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, dirName, "ExternalInstall", "ogg"));

                        BuildProject(builder, "zlib/contrib/minizip", t.Config, reposDir, dirName, generator, args);
                        BuildProject(builder, "libpng", t.Config, reposDir, dirName, generator, args);
                        BuildProject(builder, "freetype2", t.Config, reposDir, dirName, generator, args);
                        BuildProject(builder, "ogg", t.Config, reposDir, dirName, generator, args);
                        BuildProject(builder, "vorbis", t.Config, reposDir, dirName, generator, $"-DOGG_ROOT={oggInstallDir} " + args);
                    }
                }


                var targetArgs = new []
                {
                    // macOS
                    new { DirName = "macOS", Config = "Release",Args = "" },

                    // iOS
                    //new { DirName = "iOS", Args = $"-DCMAKE_TOOLCHAIN_FILE=\"{iOSToolchainFile}\" -DIOS_PLATFORM=OS" },
                };

                foreach (var t in targetArgs)
                {
                    var dirName = t.DirName;
                    var args = t.Args;
                    var zlibInstallDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, dirName, "ExternalInstall", "zlib"));
                    var oggInstallDir = Utils.ToUnixPath(Path.Combine(builder.LuminoBuildDir, dirName, "ExternalInstall", "ogg"));

                    var generator = "Xcode";
                    BuildProject(builder, "zlib", t.Config, reposDir, dirName, generator, args);
                    BuildProject(builder, "zlib/contrib/minizip", t.Config, reposDir, dirName, generator, $"-DZLIB_INCLUDE_DIR={zlibInstallDir}/include " + args);
                    BuildProject(builder, "libpng", t.Config, reposDir, dirName, generator, $"-DZLIB_INCLUDE_DIR={zlibInstallDir}/include " + args);
                    BuildProject(builder, "glslang", t.Config, reposDir, dirName, generator, args);
                    BuildProject(builder, "SPIRV-Cross", t.Config, reposDir, dirName, generator, args);
                    BuildProject(builder, "glfw", t.Config, reposDir, dirName, generator, $"-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=ON");
                    BuildProject(builder, "glad", t.Config, reposDir, dirName, generator, $"-DGLAD_INSTALL=ON " + args);
                    BuildProject(builder, "freetype2", t.Config, reposDir, dirName, generator, args);
                    BuildProject(builder, "ogg", t.Config, reposDir, dirName, generator, args);
                    BuildProject(builder, "vorbis", t.Config, reposDir, dirName, generator, $"-DOGG_ROOT={oggInstallDir} " + args);
                }
            }
        }
    }
}
