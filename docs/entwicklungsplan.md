# Entwicklungsplan

Beauftragte Erweiterungen des Weihnachts-LAN-Mods, Stand 25.10.2026. Reihenfolge ist Priorität. Jede Erweiterung bleibt ein Gamecode-Mod auf dem offiziellen Stable-Release; `spielregeln.md` wird bei Umsetzung ergänzt, nicht ersetzt.

## Leitplanken

- Spielbarkeit und Taktik bleiben Standard. Gaudi-Elemente sind zeitlich begrenzt oder per Vote zuschaltbar, nie Dauerzustand.
- Jede Erweiterung einzeln per Cvar abschaltbar; Standardwert ohne Cvar entspricht dem bisherigen Verhalten.
- Assets (Modelle, Texturen, Sounds) entstehen extern über Meshy und Blender und werden als Dateien in das Mod-PK3 aufgenommen. Code verweist nur auf Pfade; fehlende Assets dürfen nicht zum Absturz führen, sondern fallen auf Originaldateien zurück.
- Nachweis je Erweiterung: Host-Build, native Assertions, passender Smoke. Grafische Abnahme erst mit vorhandenen Assets und Original-PK3s.

## 1. Rundenmodifikatoren

- Ziel: Pro Map ein Modus, am Rundenstart angesagt. Auswahl zufällig aus einer konfigurierbaren Liste oder per Vote.
- Kandidaten, taktisch verträglich: doppelte XP, verkürzte Spawnzeit, verlängerter Dynamitzünder, Fernglas-Minensuche beschleunigt, Nebenwaffen-Runde (Pistolen und Nahkampf). Gaudi, zeitlich begrenzt: niedrige Gravitation, Messer-only.
- Einstieg: Cvars und Rundenstart in `g_main.c`; Spawn in `g_client.c`; Waffenfreigaben in `bg_classes.c`; Ansage per Server-Command an `cgame`.
- Offen: Liste der Modi, Dauer eines Gaudi-Modus, Vote-Integration.

## 2. Rundenauszeichnungen, weihnachtlich

- Ziel: Am Rundenende zusätzliche Auszeichnungen mit weihnachtlichen Titeln, etwa meiste Wiederbelebungen, meiste Entschärfungen, meiste Teamkills, meiste Minen gefunden.
- Einstieg: vorhandene Statistik in `g_stats.c`/`g_cmds.c`; Anzeige im Rundenende-Panel `cg_debriefing.c`; Vote-Einstieg für Modifikatoren in `g_vote.c`.
- Offen: Titelliste, Anzeige nur im Chat oder im Endpanel.

## 3. Nischenwaffen aufwerten

- Ziel: Mörser, MG42, Satchel und Flammenwerfer attraktiver machen, ohne Primärwaffen zu entwerten.
- Grenzen: Änderungen in `bg_pmove.c` betreffen Prediction; Client und Server gemeinsam bauen. Schadenswerte in `g_weapon.c`/`g_combat.c`, Waffentabellen in `bg_misc.c`.
- Offen: konkrete Werte; Entscheidung je Waffe nach Testrunde.

## 4. Weihnachtsoptik

- Ziel: Schneebälle statt Handgranaten, Mützen, weihnachtliche Hinweise und Sounds.
- Voraussetzung: Assets aus Meshy/Blender im ET-Format (MD3/MDC, TGA/JPG, WAV). Pfadbelegung über Shader und `.weap`-Dateien im PK3; Codeänderungen nur, wo Originaldateien nicht per PK3 überlagert werden können.
- Offen: Asset-Pipeline und Formate prüfen, Umfang festlegen, Grafiktest auf Host mit Original-PK3s.
