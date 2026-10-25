# Code-Orientierung

Die folgenden Pfade beziehen sich auf den automatisch vorbereiteten Release-Source. Sein Pfad steht unter Windows in `.cache/state.json`, unter Linux in `.cache/linux/state.json`, jeweils unter `sourcePath`. Die versionierte Änderung liegt in `mod/etlab.patch`.

| Thema | Dateien und Einstieg |
|---|---|
| Klassen, Waffen, Spawn-Loadouts | `src/game/bg_classes.c`: Klassentabellen, `BG_HeavyLoadoutAkimbo`, `BG_IsSecondaryWeaponAvailable`, `BG_GetBestSecondaryWeapon` |
| Spawn und Munition | `src/game/g_client.c`: `SetWolfSpawnWeapons`, `ClientSpawn` |
| Limbo-Auswahl | `src/cgame/cg_limbopanel.c`; Servervalidierung in `g_cmds.c` |
| Gemeinsame Waffenlogik | `src/game/bg_pmove.c`: `PM_Weapon`, Zangentakt; Client und Server gemeinsam bauen |
| Zange | `src/game/g_weapon.c`: `G_CanUsePliersOn`, `Weapon_Engineer`, Scharfmachen/Entschärfen |
| Minenflagge und Übungsfeld | `g_weapon.c`: `G_RevealLandmine`, `G_ETLabPracticeTargets`; Command in `g_cmds.c` |
| Fernglas-Minenentdeckung | `src/game/g_teammapdata.c`: `G_CheckSpottedLandMines` |
| Snapshot-Sichtbarkeit | `src/game/g_missile.c` |
| Hinweise | `src/game/g_main.c` |
| Minenschaden | `src/game/g_combat.c`: `G_DamageExt`, vorhandene Radius-/Deckungsprüfung |
| Rundenmodifikatoren, Tabelle und Pool | `src/game/bg_etlab.c`: `etlabModifiers`, `BG_ETLabModifierByName`, `BG_ETLabModifierInPool`; Enum und `CS_ETLAB_MODIFIER` in `bg_public.h` |
| Rundenmodifikatoren, Server | `src/game/g_etlab.c`: `G_ETLabModifierInit`, `G_ETLabModifierFrame`, `G_ETLabModifierSet`, `G_ETLabModifier_v`; Hooks in `g_active.c` (Gravitation, Tempo, Limbo), `g_combat.c` (Rückstoß, Kopfschuss, Vampir), `g_stats.c`, `g_cmds.c`, `g_weapon.c`, `g_teammapdata.c`, `g_client.c`; Cvars in `g_cvars.c` |
| Rundenmodifikatoren, Client | `src/cgame/cg_etlab.c`: Konfigstring, HUD-Zeile, Verstärkungstimer; Einbindung in `cg_servercmds.c`, `cg_main.c`, `cg_draw.c` |
| Rundenauszeichnungen, Tabelle | `src/game/bg_etlab.c`: `etlabAwards`; Enum `etlabAward_t` in `bg_public.h`; Zähler `etlabStat_t` und `sess.etlabStats` in `g_local.h` |
| Rundenauszeichnungen, Server | `src/game/g_etlab.c`: `G_ETLabAwardCount`, `G_ETLabAwardValue`, `G_ETLabBuildAwards`; Aufruf am Ende von `G_BuildEndgameStats` (`g_stats.c`); Zähler-Hooks in `g_weapon.c` (Revive, Entschärfen, Bauen, MG-42-Reparatur), `g_teammapdata.c`, `g_items.c`, `g_combat.c`, `g_stats.c` (`G_AddKillSkillPointsForDestruction`, Grinch); Reset in `G_deleteStats` (`g_match.c`), Persistenz in `g_session.c`; Cvar `g_etlabAwards` |
| Rundenauszeichnungen, Client | `src/cgame/cg_debriefing.c`: `CG_Debriefing_AwardIndex`, `CG_Debriefing_Awards_Draw`; `NUM_ORIGINAL_ENDGAME_AWARDS`/`NUM_ENDGAME_AWARDS` in `cg_local.h`; Chat-Zeilen `CG_ETLabAwardChat` in `cg_etlab.c`, Einstieg vor der Hash-Verteilung in `cg_servercmds.c` |
| Mod-Übersetzungen | `etmain/locale/mod/<lang>.po`, landen im Mod-PK3; Client übersetzt über `CG_TranslateString` nach `cl_lang`. Award-Texte sind msgids; neue Texte in `de.po` ergänzen |
| Deklarationen | `bg_public.h`, `g_local.h` |

Alle bisherigen Spielregeln passen in den Gamecode-Mod. Für weitere Klassen müssen zusätzlich Klassenanzahl, Tabellen, Limbo, Characters, Session, Skills, Bots und Netzwerkannahmen zusammen geprüft werden. Eine neue Klasse ist nicht automatisch ein neuer XP-Skill.

Cvars ändern nur vorhandene Optionen. Lua eignet sich für Serverregeln und Tests, soweit die API diese Funktionen zugänglich macht. Veränderungen von Waffenverhalten, Prediction oder Client-Auswahl benötigen häufig den hier verwendeten nativen Gamecode-Mod.

Engine-Dateien unter anderem in `src/client`, `src/server`, `src/renderer*` sowie externe `libs/` gehören nicht zu den Mod-Änderungen. CMake wird lediglich um das native Testziel ergänzt.
