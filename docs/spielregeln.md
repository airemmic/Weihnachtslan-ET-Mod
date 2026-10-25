# ETLab: Akimbo, Entschärfen und Minensuche

## Spielen

In diesem Mod-Projektordner **`ETLab starten.cmd` doppelklicken**. Der Starter öffnet den offiziellen Client, lädt `etlab` und startet eine lokale Partie auf `oasis`. Mit **L** Team, Klasse und Loadout wählen. Die Partie ist nur über Loopback erreichbar; es werden keine Bots gestartet.

- **2:** Pistolen; **3:** Primärwaffe; **4:** Handgranate.
- **5:** Zange, bei mehreren Werkzeugen gegebenenfalls mehrfach drücken.
- **B gedrückt halten:** Fernglas. Eine gegnerische Mine etwa 2,5 Sekunden im Blickfeld halten, bis sie markiert wird. Sichtlinie und normaler Entdeckungsfortschritt gelten weiterhin.
- **F6:** Übungsobjekte neu platzieren. Die Konsole kennt dafür auch `etlab_targets`.

Das separate Spielprofil liegt unter `profiles/play/etlab`. Die vorhandene Spielinstallation bleibt unberührt. Bestehende individuelle Tastenbelegungen gelten weiterhin; der Starter belegt F6 für das Übungsfeld.

## Spielregeln

- **Akimbo:** Soldier mit Panzerfaust, alliierter Bazooka oder Flammenwerfer erhält ab 0 XP zwei Pistolen. Die normale Freischaltung über Light Weapons Level 4 bleibt für andere Loadouts erhalten. Eine auf Heavy Weapons Level 4 gewählte zusätzliche SMG bleibt zusammen mit dem Bonus verfügbar. Der Bonus richtet sich nach dem beim Spawn gewählten Loadout; ein späterer Waffen-Pickup berechnet ihn nicht neu.
- **Zange für alle:** Soldier, Medic, Field Ops und Covert Ops können damit ausschließlich scharfes Dynamit und scharfe beziehungsweise bereits ausgelöste Landminen entschärfen. Kein Bauen, Reparieren, Scharfmachen oder Entschärfen von Satchels. Bau-/Reparaturhinweise erscheinen nur beim Engineer. Durch Entschärfen erhalten andere Klassen keine Landminen zum Legen.
- **Entschärfungsdauer:** Nicht-Engineers arbeiten bei gleichem Skill-Level halb so schnell und benötigen damit die doppelte Dauer für Dynamit und Landminen. Der gemeinsame Client-/Server-Code verwendet 100 ms statt 50 ms zwischen Zangenaktionen. Der Fortschritt pro Aktion und die jeweiligen Skill-Boni bleiben unverändert.
- **Engineer:** Behält seine normalen Fähigkeiten und seine unveränderte Geschwindigkeit beim Bauen, Reparieren, Scharfmachen und Entschärfen.
- **Fernglas:** Alle Klassen beider Teams erhalten es bereits ab 0 XP und können damit gegnerische Minen mit dem normalen Covert-Ops-Mechanismus entdecken. Dadurch werden keine Artillerie- oder Verkleidungsfähigkeiten vergeben.
- **Entdeckte Minen:** Können durch Schüsse auf das Fähnchen beziehungsweise dessen kleinen Trefferbereich oder durch Explosivschaden gesprengt werden. Das Fähnchen blockiert keine Spielerbewegung.
- **Unentdeckte, scharfe Minen:** Können durch deutsche Stielhandgranaten und alliierte Pineapple-Handgranaten gesprengt werden. Dafür muss die Mine vorher nicht markiert werden. Schüsse und andere Explosionsarten erhalten für verborgene Minen keine zusätzliche Freigabe. Nicht scharf gemachte Minen werden durch diese Erweiterung nicht zur Explosion gebracht.
- **Entfernung und Deckung:** Es gelten die vorhandenen Radius- und Deckungsprüfungen des Spiels, einschließlich dessen Nahbereichsregeln. Eine Explosion muss tatsächlich Schaden bis zur Mine übertragen. Minen explodieren mit ihrer normalen Wirkung; das ist kein gefahrloses Entfernen.
- **Skills:** Die vorhandenen XP-Vergaben und Skill-Boni bleiben wirksam; für Nicht-Engineers gilt zusätzlich der halbierte Zangentakt. Entschärfen benötigt keine Bauenergie. Der Engineer verwendet für Bauen/Reparieren weiterhin seine normalen Charge-Regeln.

## Automatisches Übungsfeld

Nur der Direktstarter aktiviert `g_etlabPractice 1`. Beim regulären Spawn des lokalen Spielers auf Oasis werden drei gegnerische, scharfe Minen und ein gegnerisches, scharfes Dynamitpaket erzeugt. Zwei Minen beginnen verborgen, eine ist bereits markiert, damit Schüsse sofort ausprobiert werden können. Der Blick wird auf die erste verborgene Mine gerichtet. **B** halten, um sie zu suchen.

Dynamit hat im Übungsfeld **fünf Minuten Zündzeit**. Das gibt Zeit zum Ausprobieren; es explodiert danach tatsächlich. F6 entfernt die eigenen Übungsobjekte samt Kartenmarkierungen und legt sie um den aktuellen Standort neu an. Bei Teamwechsel passen sich ihre Teams an. Falls ein enger Innenraum nicht genug geeigneten Boden bietet, fordert das Spiel zum Wechsel auf offenen Boden und F6 auf. Revive setzt das Übungsfeld nicht zurück.

Der Mod prüft dafür lokalen Host, Listenserver, Oasis und die explizite Cvar. Dedicated Server und normale andere Maps erhalten keine Übungsobjekte. Mit `set g_etlabPractice 0` wird das Erzeugen ausgeschaltet; eine neue Map entfernt bestehende Objekte. Das Übungsdynamit ist keinem Missionsziel zugeordnet.

## Code und Prüfungen

Es ist ein nativer Gamecode-Mod. Engine und externe Libraries wurden für diese Spielregeln nicht verändert.

- `src/game/bg_classes.c`, `bg_public.h`, `g_client.c`, `src/cgame/cg_limbopanel.c`: gemeinsame Akimbo-Auswahl, Spawn und Werkzeuge.
- `src/game/g_weapon.c`: `G_CanUsePliersOn`, tatsächlicher Zangenpfad, Minen-Scharfmachen, `G_RevealLandmine`, lokales Übungsfeld.
- `src/game/bg_pmove.c`: gemeinsamer Zangentakt für Client-Prediction und Server; Nicht-Engineers benötigen doppelt so lange.
- `src/game/g_main.c`: zulässige Interaktionshinweise.
- `src/game/g_teammapdata.c`, `g_missile.c`: Fernglassuche, Fortschritt und Sichtbarkeit.
- `src/game/g_combat.c`: zulässige Schadensarten für verborgene/entdeckte Minen; bestehende Radiusprüfung bleibt erhalten.
- `tests/native/etlab_loadouts.c` im Mod-Projekt: Native Tests mit echten Gameplay-Funktionen. Engine-Geometrie, PVS und Benachrichtigungen werden im Harness kontrolliert. Getestet werden unter anderem alle Klassen/Teams, Akimbo, Zangenaktionen, Fernglasfortschritt, Trefferbereich und Minenschaden einschließlich Deckung/Entfernung.
- `tests/mines-runtime.lua` im Workspace: echter Client-Test. Fernglas, Pistolen und Zange werden über Spielbefehle betätigt. Die tatsächlichen Zangenereignisse und vollständigen Entschärfungen werden für Soldier und Engineer bei Skill 0 und 4 zeitlich verglichen, jeweils für Dynamit und Landminen. Tatsächlich geworfene Granaten werden für reproduzierbare Positionierung testseitig neben die Mine versetzt; Zündzeit, Explosion, Radius-/Deckungsprüfung und Minenreaktion führt der echte Gamecode aus. Das Testscript wird ausschließlich in einem frischen Smoke-Profil geladen.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Build-Mod.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Test-ETLabServer.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Start-ETLabGame.ps1 -SmokeTest
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Start-ETLabGame.ps1 -MineSmoke
```

Der Buildstarter führt CTest vor dem Staging aus. Smoke-Profile sind frisch und unabhängig vom Spielprofil. Der Minentest verwendet nur dort `devmap` und Test-Cheats; der normale Direktstarter startet ohne Cheats. Logs und Screenshots liegen unter `profiles/client-smoke-*/etlab`. Eine vollständige Map-, Multiplayer-, Balance- und Revive-Abnahme bleibt eine getrennte Aufgabe.

## Verifizierter Stand

Siehe [Prüfstand](pruefstand.md) für die Abnahme dieses eigenständigen Mod-Projekts mit der offiziellen Release-Engine.
