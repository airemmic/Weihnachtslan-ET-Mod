[CmdletBinding()]
param([switch]$Locked)
. "$PSScriptRoot/Common.ps1"
foreach ($folder in @('.cache/downloads', '.cache/sources', 'runtime', 'logs')) {
    New-Item -ItemType Directory -Path (Join-Path $etlRoot $folder) -Force | Out-Null
}
$stateFile = Join-Path $etlRoot '.cache/state.json'
$previous = if (Test-Path -LiteralPath $stateFile) { Read-ETLabState } else { $null }
if ($previous) { Assert-SavedSource $previous }
$lockFile = Join-Path $etlRoot 'release.lock.json'
if ($Locked) {
    if (-not (Test-Path -LiteralPath $lockFile)) { throw 'Kein Release-Lock vorhanden.' }
    $release = Get-Content -LiteralPath $lockFile -Encoding UTF8 -Raw | ConvertFrom-Json
} else {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    $html = (Invoke-WebRequest -UseBasicParsing 'https://www.etlegacy.com/download' -TimeoutSec 30).Content
    $version = [regex]::Match($html, 'ET: Legacy stable release\s+(\d+\.\d+\.\d+)')
    $download = [regex]::Match($html, '(?s)data-href="(https://www\.etlegacy\.com/download/file/\d+)">Win64 archive</a>(.*?)md5sum:\s*([a-fA-F0-9]{32})')
    if (-not $version.Success -or -not $download.Success) { throw 'Offizielle Stable-Version/Win64-Download nicht eindeutig ermittelbar. Kein automatischer Fallback auf einen alten Stand.' }
    $release = [pscustomobject][ordered]@{
        version = $version.Groups[1].Value; tag = 'v' + $version.Groups[1].Value
        sourceUrl = 'https://github.com/etlegacy/etlegacy.git'; commit = ''
        downloadUrl = $download.Groups[1].Value; archiveMd5 = $download.Groups[3].Value.ToLowerInvariant()
        archiveSha256 = ''; policy = 'latest-published-stable'
    }
    Write-Host "Neuester offizieller Stable-Release: $($release.version)"
}
$patch = Join-Path $etlRoot 'mod/etlab.patch'
$patchHash = (Get-FileHash -LiteralPath $patch -Algorithm SHA256).Hash
$reuse = $previous -and $previous.version -eq $release.version -and $previous.patchSha256 -eq $patchHash
if ($reuse) {
    $sourceRelative = $previous.sourcePath
    $source = Join-Path $etlRoot $sourceRelative
} else {
    $sourceRelative = '.cache/sources/' + $release.tag
    $source = Join-Path $etlRoot $sourceRelative
    # Never reset or delete an old source tree. Updates get a separate checkout.
    if (Test-Path -LiteralPath $source) {
        $sourceRelative += '-' + [Guid]::NewGuid().ToString('N').Substring(0,8)
        $source = Join-Path $etlRoot $sourceRelative
    }
    Invoke-Git @('clone', '--branch', $release.tag, '--depth', '1', $release.sourceUrl, $source) | Out-Host
    if ($Locked -and $release.commit -and (Invoke-Git @('-C', $source, 'rev-parse', 'HEAD')) -ne $release.commit) { throw 'Release-Commit stimmt nicht mit Lock ueberein.' }
    Invoke-Git @('-C', $source, 'apply', '--check', $patch) | Out-Host
    Invoke-Git @('-C', $source, 'apply', $patch) | Out-Host
}
$commit = (Invoke-Git @('-C', $source, 'rev-parse', 'HEAD')).Trim()
if ($Locked -and $release.commit -and $commit -ne $release.commit) { throw 'Source-Commit stimmt nicht mit dem Release-Lock ueberein.' }
Invoke-Git @('-C', $source, 'submodule', 'update', '--init', '--recursive', '--jobs', '8') | Out-Host
New-Item -ItemType Directory -Path (Join-Path $source 'tests') -Force | Out-Null
# Tests are authored in tests/native, never in the generated checkout.
Copy-Item -Path (Join-Path $etlRoot 'tests/native/*') -Destination (Join-Path $source 'tests') -Force
$archive = Join-Path $etlRoot ('.cache/downloads/etlegacy-' + $release.version + '-win64.zip')
if (-not (Test-Path -LiteralPath $archive)) {
    $temp = $archive + '.partial'
    & curl.exe --fail --location --silent --show-error --connect-timeout 30 --max-time 600 $release.downloadUrl --output $temp
    if ($LASTEXITCODE -ne 0) { throw 'Download der offiziellen Engine fehlgeschlagen.' }
    if ((Get-FileHash -LiteralPath $temp -Algorithm MD5).Hash -ne $release.archiveMd5) { throw 'Offizielle MD5-Pruefsumme stimmt nicht. Download wird nicht benutzt.' }
    Move-Item -LiteralPath $temp -Destination $archive
}
if ((Get-FileHash -LiteralPath $archive -Algorithm MD5).Hash -ne $release.archiveMd5) { throw 'Archiv-Pruefsumme stimmt nicht.' }
$archiveHash = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash
if ($Locked -and $release.archiveSha256 -and $release.archiveSha256 -ne $archiveHash) { throw 'Archiv weicht vom SHA256 des Locks ab.' }
$runtimeRelative = 'runtime/etlegacy-v' + $release.version + '-x64'
$runtime = Join-Path $etlRoot $runtimeRelative
if (-not (Test-Path -LiteralPath (Join-Path $runtime 'etl.exe'))) {
    Expand-Archive -LiteralPath $archive -DestinationPath (Join-Path $etlRoot 'runtime')
}
foreach ($file in @('etl.exe', 'etlded.exe')) {
    if (-not (Test-Path -LiteralPath (Join-Path $runtime $file))) { throw "Offizielles Archiv hat eine unerwartete Struktur: $file fehlt." }
}
if (Test-Path -LiteralPath (Join-Path $etlRoot 'assets/etmain/pak0.pk3')) {
    foreach ($name in @('pak0.pk3', 'pak1.pk3', 'pak2.pk3')) {
        Copy-Item -LiteralPath (Join-Path $etlRoot "assets/etmain/$name") -Destination (Join-Path $runtime "etmain/$name") -Force
    }
}
$release.commit = $commit
$release.archiveSha256 = $archiveHash
Write-ETLabJson $lockFile $release
$state = [ordered]@{
    version = $release.version; sourcePath = $sourceRelative; runtimePath = $runtimeRelative
    commit = $commit; patchSha256 = $patchHash; appliedDiffSha256 = (Get-SourceDiffHash $source)
}
Write-ETLabJson $stateFile $state
Write-Host "Vorbereitet: $source"
