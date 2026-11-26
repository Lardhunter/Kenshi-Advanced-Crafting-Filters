# Building Kenshi Advanced Filters v2.0.0

Build this source with MSVC 2010 SP1 (v100), Release, x64. The release build uses the x86-hosted x64 compiler, the Visual C++ 2010 MSBuild targets, and Windows SDK 7.1 headers and x64 libraries. A compatible MSBuild host runs the project. The build uses the existing Windows PowerShell and toolchain installation.

## Obtain the dependencies

Use the [KenshiLib example dependencies](https://github.com/BFrizzleFoShizzle/KenshiLib_Examples_deps) checkout at `b566d74bf3d74629cc2fb632a97595b8202993f1`. This revision provides the KenshiLib 0.4.0 headers and the KenshiLib, MyGUI, and Ogre import libraries. The upstream checkout uses Git LFS for its library and archive files. Retrieve the LFS files and extract `boost_1_60_0\boost.zip` into `boost_1_60_0` so that `boost\version.hpp` is present there. The build uses that checkout's Boost 1.60.0 and searches its `stage\lib` directory for v100 libraries.

The dependency checkout is a build input. Its upstream code is available separately. The plugin source archive contains the versioned project and release scripts. [KenshiLib 0.4.0 source](https://github.com/BFrizzleFoShizzle/KenshiLib/tree/v0.4.0) · [Boost 1.60.0 source](https://www.boost.org/releases/1.60.0/)

## Compile and package

From the extracted source directory, replace the example paths with your installed paths. Use an output directory that does not yet exist.

```powershell
$buildPaths = @{
    MSBuildExe           = 'C:\Toolchains\MSBuild\Current\Bin\MSBuild.exe'
    VisualStudio2010Dir  = 'C:\Program Files (x86)\Microsoft Visual Studio 10.0'
    CppTargetsDir        = 'C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0'
    WindowsSdkDir        = 'C:\Program Files\Microsoft SDKs\Windows\v7.1'
    KenshiLibDir         = 'C:\Dependencies\KenshiLib_Examples_deps\KenshiLib'
    BoostDir             = 'C:\Dependencies\KenshiLib_Examples_deps\boost_1_60_0'
    OutputDirectory      = 'C:\Builds\Kenshi-Advanced-Filters-v2.0.0-build'
}
.\scripts\build-release-v100.ps1 @buildPaths
```

The build writes `bin\Organize-the-Crafting-Stations.dll` and `build.log` under the output directory. The v100 Release targets omit PDB generation. To verify the DLL, use `dumpbin /headers` for machine `8664 (x64)` and `dumpbin /exports` for `?startPlugin@@YAXXZ`. `dumpbin /dependents` lists the game and runtime DLLs.

Use a separate existing release output directory for packaging. The packaging script verifies the staged DLL's SHA-256 and creates `Kenshi-Advanced-Filters-v2.0.0.zip`. The mod folder, `.mod` file, and DLL inside retain their `Kenshi-Advanced-Crafting-Filters` names for installation compatibility.

```powershell
$builtDll = Join-Path $buildPaths.OutputDirectory 'bin\Organize-the-Crafting-Stations.dll'
.\scripts\package-release.ps1 -BuiltDll $builtDll -OutputDirectory 'C:\Releases\v2.0.0'
```

## Release provenance

The v2.0.0 tag identifies the source archive used for the final build. The release retains the in-game-tested runtime source at `3f35ee81a2ceb853023a0b2ba4791706d752fb48` and updates the version, documentation, and installation ZIP name. Build paths and linker timestamps can affect the DLL checksum when rebuilding from source.
