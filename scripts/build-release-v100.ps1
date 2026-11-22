# Build the release DLL from a source checkout or the accompanying source ZIP.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$MSBuildExe,
    [Parameter(Mandatory = $true)][string]$VisualStudio2010Dir,
    [Parameter(Mandatory = $true)][string]$CppTargetsDir,
    [Parameter(Mandatory = $true)][string]$WindowsSdkDir,
    [Parameter(Mandatory = $true)][string]$KenshiLibDir,
    [Parameter(Mandatory = $true)][string]$BoostDir,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'
$repoDir = Split-Path -Parent $PSScriptRoot
$MSBuildExe = (Resolve-Path -LiteralPath $MSBuildExe).ProviderPath
$VisualStudio2010Dir = (Resolve-Path -LiteralPath $VisualStudio2010Dir).ProviderPath
$CppTargetsDir = (Resolve-Path -LiteralPath $CppTargetsDir).ProviderPath
$WindowsSdkDir = (Resolve-Path -LiteralPath $WindowsSdkDir).ProviderPath
$KenshiLibDir = (Resolve-Path -LiteralPath $KenshiLibDir).ProviderPath
$BoostDir = (Resolve-Path -LiteralPath $BoostDir).ProviderPath
$OutputDirectory = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDirectory)
$vcDir = Join-Path $VisualStudio2010Dir 'VC'
$compilerDir = Join-Path $vcDir 'bin\x86_amd64'

$requiredFiles = @(
    $MSBuildExe
    (Join-Path $compilerDir 'cl.exe')
    (Join-Path $CppTargetsDir 'Microsoft.Cpp.Default.props')
    (Join-Path $WindowsSdkDir 'Include\Windows.h')
    (Join-Path $WindowsSdkDir 'Lib\x64\kernel32.lib')
    (Join-Path $KenshiLibDir 'Include\kenshi\util\OgreUnordered.h')
    (Join-Path $KenshiLibDir 'Libraries\KenshiLib.lib')
    (Join-Path $KenshiLibDir 'Libraries\MyGUIEngine_x64.lib')
    (Join-Path $KenshiLibDir 'Libraries\OgreMain_x64.lib')
    (Join-Path $BoostDir 'boost\version.hpp')
)
foreach ($file in $requiredFiles) {
    if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
        throw "Required build input missing: $file"
    }
}
if (Test-Path -LiteralPath $OutputDirectory) {
    throw "Use a new output directory to preserve existing builds: $OutputDirectory"
}
[void](New-Item -ItemType Directory -Path $OutputDirectory)

$buildEnvironment = @{
    PATH = "$compilerDir;$vcDir\bin;$VisualStudio2010Dir\Common7\IDE;$WindowsSdkDir\Bin;$env:PATH"
    LibraryPath = "$vcDir\lib\amd64;$WindowsSdkDir\Lib\x64;$BoostDir\stage\lib"
    KENSHILIB_DIR = $KenshiLibDir
    BOOST_INCLUDE_PATH = $BoostDir
    KENSHI_SDK_INCLUDE_PATH = "$WindowsSdkDir\Include"
}
$savedEnvironment = @{}

$buildArgs = @(
    (Join-Path $repoDir 'Organize-the-Crafting-Stations.vcxproj')
    '/t:Build'
    '/nologo'
    '/v:minimal'
    '/nr:false'
    '/p:Configuration=Release'
    '/p:Platform=x64'
    '/p:PlatformToolset=v100'
    '/p:TargetFrameworkVersion=v4.8'
    "/p:VCTargetsPath=$CppTargetsDir\"
    "/p:VCInstallDir=$vcDir\"
    "/p:VSInstallDir=$VisualStudio2010Dir\"
    "/p:WindowsSdkDir=$WindowsSdkDir\"
    "/p:ForceImportAfterCppTargets=$PSScriptRoot\v100-release.targets"
    "/p:OutDir=$OutputDirectory\bin\"
    "/p:IntDir=$OutputDirectory\obj\"
    '/fl'
    "/flp:logfile=$OutputDirectory\build.log;verbosity=normal"
)

try {
    foreach ($entry in $buildEnvironment.GetEnumerator()) {
        $savedEnvironment[$entry.Key] = [Environment]::GetEnvironmentVariable($entry.Key, 'Process')
        [Environment]::SetEnvironmentVariable($entry.Key, $entry.Value, 'Process')
    }
    & $MSBuildExe @buildArgs
    if ($LASTEXITCODE -ne 0) {
        throw "MSBuild failed with exit code $LASTEXITCODE. See $OutputDirectory\build.log."
    }
    $dll = Join-Path $OutputDirectory 'bin\Organize-the-Crafting-Stations.dll'
    if (-not (Test-Path -LiteralPath $dll -PathType Leaf)) {
        throw "MSBuild finished without creating the release DLL: $dll"
    }
    Write-Output $dll
} finally {
    foreach ($entry in $savedEnvironment.GetEnumerator()) {
        [Environment]::SetEnvironmentVariable($entry.Key, $entry.Value, 'Process')
    }
}
