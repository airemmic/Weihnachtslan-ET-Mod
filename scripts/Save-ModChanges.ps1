[CmdletBinding()]
param()
. "$PSScriptRoot/Common.ps1"
$state = Read-ETLabState
$source = Join-Path $etlRoot $state.sourcePath
$staged = @(Invoke-Git @('-C', $source, 'diff', '--cached', '--name-only'))
if ($staged.Count) { throw 'Bitte zuerst alle Aenderungen im Source-Checkout unstage; neue Dateien mit git add -N markieren.' }
$paths = @(Invoke-Git @('-C', $source, 'diff', '--name-only'))
foreach ($path in $paths) {
    if ($path -ne 'CMakeLists.txt' -and $path -notmatch '^(src/(game|cgame|ui|tvgame)/|etmain/locale/mod/)') { throw "Aenderung ausserhalb des Mod-Gamecodes: $path. Engine/Library-Aenderungen gehoeren nicht in diesen Mod." }
}
$unknown = @(Invoke-Git @('-C', $source, 'ls-files', '--others', '--exclude-standard') | Where-Object { $_ -notlike 'tests/*' })
if ($unknown.Count) { throw "Neue Moddateien zuerst mit git add -N markieren: $($unknown -join ', ')" }
$patch = Join-Path $etlRoot 'mod/etlab.patch'
$backup = Join-Path $etlRoot ('.cache/etlab-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '.patch')
Copy-Item -LiteralPath $patch -Destination $backup
Invoke-Git @('-C', $source, 'diff', '--binary', "--output=$patch") | Out-Host
$state.patchSha256 = (Get-FileHash -LiteralPath $patch -Algorithm SHA256).Hash
$state.appliedDiffSha256 = Get-SourceDiffHash $source
Write-ETLabJson (Join-Path $etlRoot '.cache/state.json') $state
Write-Host 'Gamecode-Aenderungen in mod/etlab.patch gesichert. Jetzt im aeusseren Mod-Repository reviewen und committen.'
