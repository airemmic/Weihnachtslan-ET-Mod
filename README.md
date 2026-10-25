# ETLab-Mod

Eigenständiges Entwicklungsprojekt für unseren ET: Legacy-Gamecode-Mod. Die Engine bleibt das unveränderte offizielle Release. Der Mod enthält Akimbo für schwere Loadouts, Entschärfungszangen und Fernglas für alle Klassen sowie erweiterte Minenregeln. [Alle Spielregeln](docs/spielregeln.md).

## Nach der Einrichtung starten

**`ETLab starten.cmd` doppelklicken.** Der Starter prüft zuerst die aktuelle stabile Veröffentlichung auf [etlegacy.com](https://www.etlegacy.com/download), bereitet bei Bedarf die neue Version vor, baut den Mod und öffnet Oasis mit dem Übungsfeld. Ein bereits aktueller, unveränderter Build wird wiederverwendet. F6 platziert gegnerische Übungsminen und Dynamit neu.

Das Spiel verwendet ausschließlich `runtime/` und `profiles/` in diesem Ordner. Die installierte Spielversion und ihre Einstellungen bleiben unberührt.

## Was in dieses Git-Repository gehört

| Pfad | Inhalt |
|---|---|
| `mod/etlab.patch` | Unsere Änderungen am Gamecode gegenüber dem offiziellen Release |
| `tests/native/` | Native Gameplay-Regressionstests |
| `tests/*.lua` | Server- und echte Client-Tests |
| `scripts/` | Release-Erkennung, Vorbereitung, Build, Start, Sicherung und Paketexport |
| `release.lock.json` | Zuletzt vorbereiteter Release mit Tag, Commit und Archiv-Prüfsummen |
| `docs/`, `.agents/`, `AGENTS.md` | Spielregeln, Arbeitsablauf und Code-Orientierung |

`runtime/`, `.cache/`, `assets/`, `profiles/`, `logs/`, `archive/` und `dist/` sind lokale, ignorierte Verzeichnisse. Keine Original-PK3s, komplette ET-Installation oder Buildausgaben ins Repository aufnehmen. Der automatisch vorbereitete ET-Quellcode ist eine Build-Abhängigkeit; ihr müsst keinen Engine-Fork pflegen.

## Einrichtung für deinen Freund – Windows x64

Benötigt werden Git, Windows PowerShell 5.1, `curl.exe` und Visual Studio 2022 bzw. Build Tools 2022 mit **Desktopentwicklung mit C++**, MSVC x64/x86, Windows SDK und C++-CMake-Tools. Beim Erstbuild ist Internetzugang erforderlich.

1. Dieses Mod-Repository klonen und in den Projektordner wechseln:

   ```powershell
   git clone https://github.com/tschinagl/ETLab-Mod.git
   cd ETLab-Mod
   ```
2. Die Original-Spieldaten aus einer vorhandenen ET-Installation importieren:

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Import-GameAssets.ps1 -Source 'C:\Pfad\zu\ETLegacy\etmain'
   ```

3. `ETLab starten.cmd` doppelklicken. Beim ersten Mal werden offizieller Source und Engine heruntergeladen und der Mod gebaut.

Die Entwickler benötigen die Build-Tools. Spieler benötigen nur eine passende offizielle Spielinstallation und das fertige Mod-Paket.

## Gemeinsam entwickeln

Beide arbeiten an diesem kleinen Mod-Repository, jeweils auf einem eigenen Branch. Eine Änderung gemeinsam beschreiben, umsetzen, testen und über einen Pull Request zusammenführen. Das gemeinsame öffentliche Repository ist [tschinagl/ETLab-Mod](https://github.com/tschinagl/ETLab-Mod).

1. `Mod bauen.cmd` ausführen: aktuelle stabile Basis vorbereiten.
2. Den von `Prepare-Mod.ps1` ausgegebenen Source-Ordner bearbeiten. Der aktive Pfad steht zusätzlich in `.cache/state.json` unter `sourcePath`. Darin liegen die vertrauten Dateien `src/game/`, `src/cgame/` usw. [Code-Orientierung](docs/code-landkarte.md).
3. Änderungen in den versionierten Mod übernehmen:

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Save-ModChanges.ps1
   ```

   Das aktualisiert `mod/etlab.patch`. Neue C-Dateien vorher im Source-Checkout mit `git add -N <Datei>` erfassen. Die nativen Tests direkt in `tests/native/` dieses Mod-Repositorys bearbeiten. Nichts im generierten `tests/`-Verzeichnis ändern.

4. `Mod bauen.cmd` ausführen und die passenden Runtime-Tests starten:

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Test-ETLabServer.ps1
   powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Start-ETLabGame.ps1 -MineSmoke
   ```

5. Im **äußeren Mod-Repository** Patch, Tests, Doku und gegebenenfalls `release.lock.json` reviewen und committen. Den Abhängigkeits-Checkout unter `.cache/` nicht als euer Repository verwenden.

Vor Updates schützt das Skript ungesicherte Gamecode-Änderungen durch Abbruch. `Save-ModChanges.ps1` legt zusätzlich eine lokale Patch-Sicherung unter `.cache/` ab. Nach einem `git pull` mit geändertem Patch entsteht ein neuer vorbereiteter Source-Ordner; alte Ordner bleiben erhalten.

## Immer die aktuellste veröffentlichte Version

Normales Bauen und der Doppelklick-Starter prüfen jedes Mal den **neuesten offiziellen Stable-Release**. Kein `master`, Nightly oder Release Candidate. Stand der Einrichtung: **2.86.2**. Das Skript liest Version, Windows-x64-Download und MD5 von der offiziellen Downloadseite, prüft das Archiv und hält zusätzlich SHA256 und Source-Commit fest.

Ein neues Release wird in einem separaten Source- und Runtime-Verzeichnis vorbereitet. Passt der Mod-Patch nicht mehr, stoppt das Skript vor dem Build. Dann muss unser Mod an den neuen Release angepasst werden; das lässt sich für unbekannte künftige Releases nicht automatisch garantieren. Der bisherige Stand bleibt erhalten. Auch bei fehlender Verbindung oder geändertem Webseitenformat gibt es einen Fehler statt eines stillen Starts mit einem alten Release.

Für einen bewusst reproduzierbaren Test des gemeinsam eingecheckten Stands gibt es eine ausdrückliche Ausnahme:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Mod.ps1 -Locked
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Start-Mod.ps1 -Locked
```

`-Locked` verwendet `release.lock.json`; es fragt nicht nach einem neueren Release. Bereits vorhandene Abhängigkeiten können damit offline verwendet werden. Neue Rechner brauchen auch dafür die Downloads. Die direkten `Start-ETLabGame`-/`Server`-/`Client`-Skripte sind Testwerkzeuge für den bereits gebauten Stand; für den normalen Start `ETLab starten.cmd` verwenden.

## Fertiges Paket

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Export-Mod.ps1
```

Erzeugt unter `dist/` ein Windows-x64-Mod-ZIP und ein ZIP mit diesem Mod-Projekt inklusive Release-Lock und Buildanleitung. Das Paket enthält keine Engine und keine Original-Spieldaten. Für einen Linux-Server werden Linux-Mod-Binaries benötigt; dieses Projekt automatisiert und prüft derzeit Windows x64. Es wurde kein bestehender Server umgestellt.

[Installation des fertigen Mods](docs/installation.md) · [Prüfumfang und Ergebnisse](docs/pruefstand.md)

## Lizenz und Herkunft

Der Gamecode basiert auf [ET: Legacy](https://github.com/etlegacy/etlegacy). Die übernommenen Copyright-Vermerke bleiben erhalten; der Mod-Source wird mit der GPL-3.0-Lizenz in `LICENSE` bereitgestellt. Original-Spieldaten sind davon getrennt und nicht Bestandteil dieses Repositorys oder der erzeugten Pakete. Für eine Weitergabe das Mod-Projekt samt passendem Release-Lock und Buildskripten mitliefern; die verwendeten Upstream-Quellen sind im Lock konkret bezeichnet und werden durch die Skripte bezogen.
