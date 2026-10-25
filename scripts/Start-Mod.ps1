[CmdletBinding()]
param([switch]$Locked)
$ErrorActionPreference = 'Stop'
& "$PSScriptRoot/Build-Mod.ps1" -Locked:$Locked -OnlyIfNeeded
& "$PSScriptRoot/Start-ETLabGame.ps1"
