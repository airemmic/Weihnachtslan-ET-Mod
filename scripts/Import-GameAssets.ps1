[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Source)
$ErrorActionPreference = 'Stop'
$etlRoot = Split-Path $PSScriptRoot -Parent
$destination = Join-Path $etlRoot 'assets\etmain'
New-Item -ItemType Directory -Path $destination -Force | Out-Null
foreach ($name in @('pak0.pk3', 'pak1.pk3', 'pak2.pk3')) {
    $sourceFile = Join-Path $Source $name
    $targetFile = Join-Path $destination $name
    if (-not (Test-Path -LiteralPath $sourceFile)) { throw "Originaldatei fehlt: $sourceFile" }
    $sourceHash = (Get-FileHash -LiteralPath $sourceFile -Algorithm SHA256).Hash
    if (Test-Path -LiteralPath $targetFile) {
        if ((Get-FileHash -LiteralPath $targetFile -Algorithm SHA256).Hash -eq $sourceHash) { Write-Host "$name bereits identisch"; continue }
        throw "Abweichende Zieldatei wird nicht ueberschrieben: $targetFile"
    }
    Copy-Item -LiteralPath $sourceFile -Destination $targetFile
    if ((Get-FileHash -LiteralPath $targetFile -Algorithm SHA256).Hash -ne $sourceHash) { throw "Kopierpruefung fehlgeschlagen: $name" }
    Write-Host "$name kopiert und SHA256 verifiziert"
}

Write-Host 'Spieldaten importiert. Build-Mod.ps1 stellt sie beim naechsten Lauf bereit.'
