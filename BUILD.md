# Building Kenshi Advanced Crafting Filters v1.0.1

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
    OutputDirectory      = 'C:\Builds\Kenshi-Advanced-Crafting-Filters-v1.0.1-build'
}
.\scripts\build-release-v100.ps1 @buildPaths
```

The build writes `bin\Organize-the-Crafting-Stations.dll` and `build.log` under the output directory. The v100 Release targets omit PDB generation. To verify the DLL, use `dumpbin /headers` for machine `8664 (x64)` and `dumpbin /exports` for `?startPlugin@@YAXXZ`. `dumpbin /dependents` lists the game and runtime DLLs.

Use a separate existing release output directory for packaging. The packaging script gives the copied DLL its release name and verifies the SHA-256 before creating the ZIP.

```powershell
$builtDll = Join-Path $buildPaths.OutputDirectory 'bin\Organize-the-Crafting-Stations.dll'
.\scripts\package-release.ps1 -BuiltDll $builtDll -OutputDirectory 'C:\Releases\v1.0.1'
```

## Release provenance

The in-game v1.0.1 test used the runtime source at `62d92c42e66a0244ba9ddd4c36f171d23f0a77fe`. The release uses the same runtime source and build scripts. The installed test DLL and saved test build had the following SHA-256.

```text
1BCB67944EF0DB19CD36E81CAF8D9F373B4629BA7B51EC44C93A78FF0E9A7A6E
```
