# Linux-Toolvertrag für LLM-Agenten

## Ausführungsgrenze

Reguläre Arbeit **nur auf einem Linux-Arbeitsrechner**. Auf Windows keine Linux-Prüfung über WSL, Container, VM oder Remote-Host starten. Eine plattformübergreifende Abnahme ist Teil eines ausdrücklich beauftragten Releases. Diese Referenz ist keine Freigabe, eine zweite Testplattform zu aktivieren.

## Voraussetzungen und Abhängigkeiten

- Ziel: Linux x86_64 mit glibc. Python ≥ 3.10; `scripts/etlab-linux.py` verwendet nur die Standardbibliothek.
- Toolchain: Git, GCC/G++, Make, CMake, Ninja, pkg-config. Die zum Release gehörenden Libraries werden als Submodule bezogen und unverändert verwendet.
- Ubuntu/Debian-Paketnamen für eine beauftragte Einrichtung: `build-essential cmake ninja-build pkg-config python3 git ca-certificates zip`.
- Client-Runtime: Desktop-Sitzung, OpenGL, Original-PK3s. Dedicated Server: keine grafische Oberfläche erforderlich.
- Auf anderen Distributionen äquivalente Pakete konkret prüfen. Keine Aussage über alle Distributionen aus einem Ubuntu-Build ableiten.

## CLI-Vertrag

Vom Repository-Root aus:

| Kommando | Effekt / Nachbedingung |
|---|---|
| `python3 scripts/etlab-linux.py prepare` | Neuester Stable-Release; geschützter Source-Checkout; offizielle Engine; Lock und State geschrieben |
| `python3 scripts/etlab-linux.py import-assets --source /pfad/etmain` | pak0/1/2 lokal kopiert und SHA256-geprüft; abweichende vorhandene Assets nicht ersetzt |
| `python3 scripts/etlab-linux.py build` | Mod-only Release-Build, CTest, dann Staging; Engine nicht kompiliert |
| `python3 scripts/etlab-linux.py save` | Gameplay-Diff nach `mod/etlab.patch`; vorherige Patch-Datei lokal gesichert |
| `python3 scripts/etlab-linux.py start` | Latest-Prüfung, erforderlicher Build, lokales Oasis-Übungsfeld |
| `python3 scripts/etlab-linux.py smoke` | Offizieller Dedicated Server: Oasis, Lua, Loopback, RCON, sauberer Exit |
| `python3 scripts/etlab-linux.py start --locked --mine-smoke` | Echter Client, frisches Profil, vorhandene Lua-Gameplay-Assertions, Screenshot, Prozessende |
| `python3 scripts/etlab-linux.py server --locked --bind 0.0.0.0 --port 27960` | Expliziter LAN-Server; nur im Bereitstellungsauftrag ausführen |
| `python3 scripts/etlab-linux.py connect --locked --address HOST:PORT` | Client mit vorbereitetem Mod-Stand verbinden |
| `python3 scripts/etlab-linux.py export` | Linux-Mod-ZIP des festgehaltenen Releases unter `dist/` |
| `python3 scripts/etlab-linux.py export --windows-package /pfad/windows-mod.zip` | Passende Windows-Binaries und Linux-Binaries in gemeinsames Paket kombinieren |

`--locked` bei prepare/build/start/server/connect: `release-linux.lock.json` statt Latest-Abfrage. Export verwendet immer den Lock. `--jobs 1..32` bei build/start/server/connect/export. Server ohne `--bind`: nur `127.0.0.1`. Starter richten keinen Downloadserver ein und schalten keine Firewall frei.

## Persistierter Zustand

- State: `.cache/linux/state.json`; aktiver Checkout aus `sourcePath`, Engine-Verzeichnis aus `runtimePath` bestimmen.
- Regulärer Build: `<sourcePath>/build`; kein Source- oder Build-Verzeichnis mit Windows teilen.
- Native Artefakte: `qagame.mp.x86_64.so`, `cgame.mp.x86_64.so`, `ui.mp.x86_64.so`, `tvgame.mp.x86_64.so`, ein Mod-PK3.
- Profile: `profiles/linux-*`; Nachweise: `logs/linux/` und jeweilige Profil-Logs/Screenshots.
- Buildmanifest speichert Input-Fingerprint und Output-Hashes. Unveränderte Builds werden wiederverwendet.
- Vor Updates Source-Diff prüfen. Ungesicherte/staged/unbekannte Dateien → Abbruch; keine automatische Löschung. Laufende Runtime-Prozesse vor einem Neubau melden statt beenden.

## Paketvertrag

- Windows-ZIP muss denselben Source-Commit und Mod-Patch enthalten. Dateihashes vor Übernahme prüfen.
- Kombiniertes PK3 enthält Windows- und Linux-Clientmodule. Dasselbe kombinierte ZIP auf allen LAN-Teilnehmern bereitstellen; getrennte plattformspezifische PK3s nicht unter gleichem Namen mischen.
- Export ersetzt keine lokalen Einzelplattform-Runtimes. Installation und Release-Abnahme: `installation.md`.
- Linux-Binaries können glibc-Symbole des Buildsystems benötigen; Manifest nennt Buildsystem und libc. Für ältere Distributionen auf passender Basis bauen; nicht pauschal Linux-Kompatibilität zusichern.

## Referenzen

- Offizielle Release-Metadaten: https://www.etlegacy.com/download
- Upstream-Linux-Installation: https://github.com/etlegacy/etlegacy/wiki/Linux
- Upstream-Source/Build: https://github.com/etlegacy/etlegacy
- Testauswahl und Nachweisgrenzen: `pruefstand.md`
