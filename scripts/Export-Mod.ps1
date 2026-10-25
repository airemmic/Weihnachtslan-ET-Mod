[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
& "$PSScriptRoot/Build-Mod.ps1" -Locked -OnlyIfNeeded
. "$PSScriptRoot/Common.ps1"
$state = Read-ETLabState
$runtime = Join-Path $etlRoot $state.runtimePath
$id = 'etlab-win64-' + $state.version + '-' + (Get-Date -Format 'yyyyMMdd-HHmmss')
$stage = Join-Path $etlRoot ('dist/' + $id)
New-Item -ItemType Directory -Path (Join-Path $stage 'etlab') -Force | Out-Null
Copy-Item -Path (Join-Path $runtime 'etlab/*.dll'), (Join-Path $runtime 'etlab/*.pk3') -Destination (Join-Path $stage 'etlab')
Copy-Item -LiteralPath (Join-Path $etlRoot 'LICENSE'), (Join-Path $etlRoot 'docs/installation.md') -Destination $stage
$files = @(Get-ChildItem -LiteralPath (Join-Path $stage 'etlab') -File | ForEach-Object { [ordered]@{name=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash} })
Write-ETLabJson (Join-Path $stage 'manifest.json') ([ordered]@{mod='etlab'; platform='windows-x64'; etLegacy=$state.version; commit=$state.commit; patchSha256=$state.patchSha256; files=$files})
Compress-Archive -Path (Join-Path $stage '*') -DestinationPath ($stage + '.zip')
# Explicit allow-list excludes all assets, profiles, binaries, caches and Git metadata.
$projectStage = Join-Path $etlRoot ('dist/' + $id + '-project')
New-Item -ItemType Directory -Path $projectStage -Force | Out-Null
foreach ($name in @('mod','tests','scripts','docs','.agents','AGENTS.md','README.md','LICENSE','release.lock.json','.gitignore','.gitattributes','ETLab starten.cmd','Mod bauen.cmd')) {
    Copy-Item -LiteralPath (Join-Path $etlRoot $name) -Destination $projectStage -Recurse
}
# tar includes dotfiles, unlike some Compress-Archive versions.
& tar.exe -a -cf ($projectStage + '.zip') -C $projectStage .
if ($LASTEXITCODE -ne 0) { throw 'Projekt-ZIP konnte nicht erstellt werden.' }
Write-Host "Mod-Paket: $stage.zip"
Write-Host "Mod-Projekt: $projectStage.zip"
