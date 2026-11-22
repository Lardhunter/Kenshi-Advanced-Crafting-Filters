# Stage the end-user mod folder and ZIP from a successful release build.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$BuiltDll,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'
$repoDir = Split-Path -Parent $PSScriptRoot
$modName = 'Kenshi-Advanced-Crafting-Filters'
$version = (Get-Content -LiteralPath (Join-Path $repoDir 'VERSION') -Raw).Trim()
if ($version -notmatch '^\d+\.\d+\.\d+$') {
    throw 'VERSION must contain the numeric release version.'
}

$BuiltDll = (Resolve-Path -LiteralPath $BuiltDll).ProviderPath
$OutputDirectory = (Resolve-Path -LiteralPath $OutputDirectory).ProviderPath
$templateDir = Join-Path $repoDir 'Organize-the-Crafting-Stations'
$modDir = Join-Path $OutputDirectory $modName
$archive = Join-Path $OutputDirectory "$modName-v$version.zip"
if ((Test-Path -LiteralPath $modDir) -or (Test-Path -LiteralPath $archive)) {
    throw 'The staged mod directory or release ZIP already exists.'
}

[void](New-Item -ItemType Directory -Path $modDir)
$pluginFile = "$modName.dll"
$stagedDll = Join-Path $modDir $pluginFile
Copy-Item -LiteralPath $BuiltDll -Destination $stagedDll
Copy-Item -LiteralPath (Join-Path $templateDir 'Organize-the-Crafting-Stations.mod') -Destination (Join-Path $modDir "$modName.mod")
Copy-Item -LiteralPath (Join-Path $templateDir 'mod-config.json') -Destination $modDir
foreach ($file in @('README.md', 'CHANGELOG.md', 'LICENSE', 'NOTICE.md')) {
    Copy-Item -LiteralPath (Join-Path $repoDir $file) -Destination $modDir
}
$manifest = @{ Plugins = @($pluginFile) } | ConvertTo-Json
[IO.File]::WriteAllText((Join-Path $modDir 'RE_Kenshi.json'), $manifest + [Environment]::NewLine)
if ((Get-FileHash -LiteralPath $BuiltDll).Hash -ne (Get-FileHash -LiteralPath $stagedDll).Hash) {
    throw 'The staged DLL differs from the build.'
}

Compress-Archive -LiteralPath $modDir -DestinationPath $archive -CompressionLevel Optimal
Write-Output $archive
