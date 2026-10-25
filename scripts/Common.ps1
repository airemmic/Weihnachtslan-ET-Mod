$ErrorActionPreference = 'Stop'
$etlRoot = Split-Path $PSScriptRoot -Parent
function Invoke-Git {
    param([string[]]$Arguments)
    $old = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try { & git @Arguments; $code = $LASTEXITCODE } finally { $ErrorActionPreference = $old }
    if ($code -ne 0) { throw "Git fehlgeschlagen (Exit $code): $($Arguments -join ' ')" }
}
function Read-ETLabState {
    $file = Join-Path $etlRoot '.cache/state.json'
    if (-not (Test-Path -LiteralPath $file)) { throw 'Zuerst scripts/Build-Mod.ps1 ausfuehren.' }
    return (Get-Content -LiteralPath $file -Encoding UTF8 -Raw | ConvertFrom-Json)
}
function Write-ETLabJson {
    param([string]$Path, $Value)
    $json = ($Value | ConvertTo-Json -Depth 8) -replace "`r?`n", "`r`n"
    [IO.File]::WriteAllText($Path, $json + "`r`n", (New-Object Text.UTF8Encoding($false)))
}
function Get-SourceDiffHash {
    param([string]$Source)
    $out = Join-Path $etlRoot 'logs/source-current.patch'
    Invoke-Git @('-C', $Source, 'diff', '--binary', "--output=$out") | Out-Host
    return (Get-FileHash -LiteralPath $out -Algorithm SHA256).Hash
}
function Assert-SavedSource {
    param($State)
    $source = Join-Path $etlRoot $State.sourcePath
    if ((Get-SourceDiffHash $source) -ne $State.appliedDiffSha256) {
        throw 'Ungesicherte Gamecode-Aenderungen: zuerst scripts/Save-ModChanges.ps1 ausfuehren. Es wurde nichts ueberschrieben.'
    }
    $staged = @(Invoke-Git @('-C', $source, 'diff', '--cached', '--name-only'))
    if ($staged.Count -gt 0) { throw 'Staged Aenderungen im Abhaengigkeits-Checkout: bitte dort unstage und Save-ModChanges ausfuehren.' }
    $unknown = @(Invoke-Git @('-C', $source, 'ls-files', '--others', '--exclude-standard') | Where-Object { $_ -notlike 'tests/*' })
    if ($unknown.Count -gt 0) { throw "Neue Dateien im Source-Checkout zuerst mit git add -N markieren und Save-ModChanges ausfuehren: $($unknown -join ', ')" }
}
