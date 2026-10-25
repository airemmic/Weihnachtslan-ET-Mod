# Prüfstand – Windows x64

Geprüft am 25.10.2026 gegen offiziellen ET: Legacy-Stable-Release 2.86.2, Source-Commit `fdb40f8706b4e5081bf721cdc5726b2791a6e1f9`.

- Mod-only Release-Build mit Visual Studio 2022 / MSVC 19.44 erfolgreich. `BUILD_CLIENT=OFF`, `BUILD_SERVER=OFF`, `BUILD_MOD=ON`.
- 793 native Gameplay-Prüfungen erfolgreich: beide Teams, alle Klassen/Loadouts, XP 0/4, Zangenrechte, Minenentdeckung und Schaden.
- Offizielle `etl.exe` und `etlded.exe` stimmen bytegenau mit dem geprüften offiziellen Win64-Archiv überein. Keine selbst gebaute Engine.
- Dedicated-Server-Smoke erfolgreich: Mod geladen, Oasis, Lua-Init und 20 Frames, Loopback-Bindung, RCON und sauberer Exit.
- Echter Client-Test erfolgreich: Akimbo, Fernglas-Minenentdeckung, Schüsse auf markierte Minen, komplette Entschärfung beider Sprengmittel und beide Handgranaten gegen verborgene Minen. Screenshot geprüft.
- Zangentakt Soldier/Engineer bei Skill 0 und 4: etwa 100/50 ms, mit Frame-Auflösung. Der Test vergleicht die vollständigen Entschärfungen mit passender Frame-Toleranz. Der Engineer bleibt unverändert.
- Schutz vor ungesicherten Source-Änderungen gezielt geprüft: Vorbereitung stoppt, Änderungen bleiben erhalten.

Granaten werden im Client-Test nach dem tatsächlichen Wurf für reproduzierbare Positionierung neben die Mine versetzt; die Explosion und Schadensermittlung sind echte Spielausführung. Die nativen Tests verwenden kontrollierte Engine-Geometrie/PVS. Diese Nachweise ersetzen keine vollständige Multiplayer-, Map-, Balance-, Revive- oder Bot-Abnahme.

Upstream-Compilerwarnungen sind vorhanden. Linux/macOS, andere Architekturen und eine bestehende entfernte Serverinstanz sind nicht abgenommen. Neue Releases müssen den Build und anschließend die passenden Runtime-Tests erneut bestehen.

Lokale Nachweise: `logs/`, `profiles/client-smoke-*/etlab/etconsole.log`, zugehörige Screenshots und `.cache/sources/*/build/Testing/Temporary/LastTest.log`. Download und Source-Basis stehen in `release.lock.json`.
