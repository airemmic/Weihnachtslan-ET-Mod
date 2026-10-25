# Entwicklungsplan

Beauftragte Erweiterungen des Weihnachts-LAN-Mods, Stand 25.10.2026. Reihenfolge ist Priorität. Jede Erweiterung bleibt ein Gamecode-Mod auf dem offiziellen Stable-Release; `spielregeln.md` wird bei Umsetzung ergänzt, nicht ersetzt.

## Leitplanken

- Spielbarkeit und Taktik bleiben Standard. Gaudi-Elemente sind zeitlich begrenzt oder per Vote zuschaltbar, nie Dauerzustand.
- Jede Erweiterung einzeln per Cvar abschaltbar; Standardwert ohne Cvar entspricht dem bisherigen Verhalten.
- Assets (Modelle, Texturen, Sounds) entstehen extern über Meshy und Blender und werden als Dateien in das Mod-PK3 aufgenommen. Code verweist nur auf Pfade; fehlende Assets dürfen nicht zum Absturz führen, sondern fallen auf Originaldateien zurück.
- Nachweis je Erweiterung: Host-Build, native Assertions, passender Smoke. Grafische Abnahme erst mit vorhandenen Assets und Original-PK3s.
- Nach jedem abgeschlossenen Punkt zusätzlich den Projektartikel in der Wissensdatenbank nachziehen (Space „WD - Programmieren“, Artikel 4519): Nachweise, Todo-Liste, Verlauf. Nur belegte Ergebnisse eintragen.

## Stand und Übergabe

Stand 25.10.2026. Bei jeder Änderung an Status oder Branch-Lage diesen Abschnitt nachziehen.

- **Ein Mod, ein Repository:** Alle Erweiterungen sind Teile desselben Mods `etlab` (ein `fs_game`, gemeinsame Module, ein `mod/etlab.patch`). Ein Server lädt nur einen Mod; getrennte Mods oder Repositories könnten nicht gemeinsam laufen. Jede Erweiterung bleibt per Cvar einzeln schaltbar.
- **Branch-Lage:** Aktueller Stand auf `ccr-ad00a20d-4uwhgx`; enthält `ccr-78a33ed0-k5azh0` vollständig. Beide sind noch nicht in `main`. Nach Übernahme nach `main` ist `ccr-78a33ed0-k5azh0` überholt.
- **Arbeitsweise:** Erweiterungen nacheinander entwickeln und zügig nach `main` übernehmen. Parallele Branches kollidieren in der einen Patch-Datei; Konflikte im Source-Checkout lösen und per `save` neu erzeugen, nie im Patch von Hand mischen.
- **LAN-Betrieb:** Ohne Cvars verhält sich der Mod wie vor den Erweiterungen. Für die LAN mindestens `g_etlabModifierPool`, `g_etlabModifierMode` und `g_etlabAwards 1` am Server setzen; siehe `installation.md`.

| Punkt | Code und native Tests | Offen |
|---|---|---|
| 1. Rundenmodifikatoren | fertig | Runtime-Abnahme mit Original-PK3s, Werte nach Testrunde justieren |
| 2. Rundenauszeichnungen | fertig, inklusive Deutsch/Englisch nach `cl_lang` | Runtime-Abnahme (Endpanel, Chat, Übersetzung, Zähler im echten Spiel); `Save-ModChanges.ps1` mit `etmain/locale/mod/`-Änderung einmal unter Windows ausführen |
| 3. Nischenwaffen | nicht begonnen | Werte je Waffe festlegen |
| 4. Weihnachtsoptik | nicht begonnen | Asset-Pipeline, Umfang |

## 1. Rundenmodifikatoren

- Ziel: Pro Map ein Modus, am Rundenstart angesagt. Auswahl zufällig aus einer konfigurierbaren Liste oder per Vote.
- Kandidaten, taktisch verträglich: doppelte XP, verkürzte Spawnzeit, verlängerter Dynamitzünder, Fernglas-Minensuche beschleunigt, Nebenwaffen-Runde (Pistolen und Nahkampf). Gaudi, zeitlich begrenzt: niedrige Gravitation, Messer-only.
- Einstieg: Cvars und Rundenstart in `g_main.c`; Spawn in `g_client.c`; Waffenfreigaben in `bg_classes.c`; Ansage per Server-Command an `cgame`.
- Entschieden, zwei getrennte Einstellungen vorab durch den Serverbetreiber:
  - `g_etlabModifierPool`: Liste der Modi, die überhaupt zur Wahl stehen. Nur Modi aus dem Pool werden gezogen oder angeboten.
  - `g_etlabModifierMode`: Art der Wahl aus dem Pool. 0 aus, 1 Zufall pro Map, 2 Vote durch die Spieler, 3 Zufall mit Vote-Übersteuerung.
- Beide Cvars zur Laufzeit änderbar, wirksam ab der nächsten Map. Ein leerer Pool bedeutet keine Modifikatoren.
- Umgesetzt: 16 Modi (10 taktisch, 6 Gaudi), Pool- und Modus-Cvars, Vote, HUD-Zeile, native Tests. Sollverhalten in `spielregeln.md`. Gaudi-Dauer `g_etlabModifierMinutes`, Standard 5.
- Offen: Runtime-Abnahme auf Host mit Original-PK3s (HUD-Zeile, Vote-Ablauf, Headshots-Only, Rückstoß, Gravitation); Werte nach Testrunde feinjustieren.

## 2. Rundenauszeichnungen, weihnachtlich

- Ziel: Am Rundenende zusätzliche Auszeichnungen mit weihnachtlichen Titeln, etwa meiste Wiederbelebungen, meiste Entschärfungen, meiste Teamkills, meiste Minen gefunden.
- Entschieden: zehn Auszeichnungen (inklusive Grinch); Sprache nach `cl_lang` des Spielers, Deutsch über `locale/mod/de.po`, sonst Englisch; Anzeige im Debriefing-Endpanel und als Chat-Ansage; Cvar `g_etlabAwards`, Standard 0.
- Umgesetzt: Zähler in der Session-Statistik, Vergabe in `G_BuildEndgameStats` angehängt, Endpanel mit Weihnachts-Awards zuerst, native Tests. Sollverhalten in `spielregeln.md`.
- Offen: Runtime-Abnahme auf Host mit Original-PK3s (Endpanel, Scrollen, Chat-Zeilen, Revive-/Pack-/Konstruktionszähler im echten Spiel).

## 3. Nischenwaffen aufwerten

- Ziel: Mörser, MG42, Satchel und Flammenwerfer attraktiver machen, ohne Primärwaffen zu entwerten.
- Grenzen: Änderungen in `bg_pmove.c` betreffen Prediction; Client und Server gemeinsam bauen. Schadenswerte in `g_weapon.c`/`g_combat.c`, Waffentabellen in `bg_misc.c`.
- Offen: konkrete Werte; Entscheidung je Waffe nach Testrunde.

## 4. Weihnachtsoptik

- Ziel: Schneebälle statt Handgranaten, Mützen, weihnachtliche Hinweise und Sounds.
- Voraussetzung: Assets aus Meshy/Blender im ET-Format (MD3/MDC, TGA/JPG, WAV). Pfadbelegung über Shader und `.weap`-Dateien im PK3; Codeänderungen nur, wo Originaldateien nicht per PK3 überlagert werden können.
- Offen: Asset-Pipeline und Formate prüfen, Umfang festlegen, Grafiktest auf Host mit Original-PK3s.
