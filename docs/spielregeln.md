# Gameplay-Vertrag für LLM-Agenten

Zweck: Sollverhalten bei Änderungen und Tests erhalten. Regeln gelten für beide Teams, sofern nicht ausdrücklich eingeschränkt. Diese Datei beschreibt Gameplay; Build- und Plattformregeln stehen in `AGENTS.md` und `llm-workflow.md`.

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

## Rundenmodifikatoren

- **Konfiguration:** `g_etlabModifierPool` nennt die wählbaren Modi (`all` oder Namen, getrennt durch Leerzeichen, Komma oder Semikolon). `g_etlabModifierMode`: 0 aus, 1 Zufall pro Map, 2 nur Vote, 3 Zufall mit Vote-Übersteuerung. `g_etlabModifierMinutes` (Standard 5) begrenzt die Dauer der Gaudi-Modi. Leerer Pool oder Modus 0 bedeutet keine Modifikatoren.
- **Auswahl:** Zufall zieht beim Mapstart gleichverteilt aus dem Pool. `callvote modifier <name|none>` ist nur in Modus 2 und 3 erlaubt (Referees immer) und akzeptiert nur Pool-Mitglieder; ein erfolgreicher Vote gilt sofort. Der aktive Modus wird in `etlab_modifier` gehalten und überlebt `map_restart` (Warmup-Ende, Stopwatch-Runde 2), solange er im Pool bleibt. Mapwechsel wählt neu.
- **Ansage:** Center-Print beim Beitritt und bei Aktivierung, Konfigstring `CS_ETLAB_MODIFIER` für das HUD (Titel, Restzeit).
- **Taktische Modi, ganze Map:** `doublexp` doppelte XP; `fastspawn` Verstärkungszeit halbiert (Server und Client-Timer); `longdyno` Dynamit 45 statt 30 Sekunden; `quickspot` Fernglas-Minensuche doppelt so schnell; `medicshortage` höchstens zwei Medics je Team, Medic-Ladezeit ×1,5; `engineerfest` Minenlimit verdoppelt, Engineer-Ladezeit halbiert; `heavyfree` keine Teamlimits für schwere Waffen; `lowammo` Reservemunition beim Spawn halbiert, Handgranaten und Magazin unverändert; `suddendeath` keine Verstärkung in den letzten zwei Minuten eines Timelimits; `classrotation` wer mit derselben Klasse wie beim letzten Spawn erneut spawnt, erhält die nächste Klasse, die die Team-Klassenlimits (`team_max*`) zulassen, mit deren Standard-Loadout; ist keine andere Klasse frei, bleibt die Klasse.
- **Gaudi-Modi, zeitlich begrenzt ab Matchstart:** `lowgravity` halbe Schwerkraft; `sidearms` keine Primärwaffen und Handgranaten, Klassenwerkzeuge inklusive Dynamit und Minen bleiben; `iceskating` Laufgeschwindigkeit ×1,4, Rückstoß verdoppelt; `grenaderain` Handgranaten alle 20 Sekunden aufgefüllt; `headshots` Spieler verletzen Spieler nur per Kopfschuss, Rückstoß bleibt; `vampire` jeder Gegner-Kill heilt 20 Lebenspunkte bis zum Maximum.
- **Invarianten:** Ohne aktiven Modus entsprechen alle Werte dem bisherigen Verhalten. Ladezeiten werden samt Konfigstring neu gesetzt; Map-Script-Faktoren bleiben multiplikativ erhalten. Nicht-scharfe Werte wie das Minenlimit `-1` bleiben unverändert.

## Rundenauszeichnungen

- **Konfiguration:** `g_etlabAwards 1` aktiviert die weihnachtlichen Auszeichnungen. Standard `0` entspricht dem bisherigen Rundenende ohne zusätzliche Einträge und Chat-Zeilen.
- **Anzeige:** Am Rundenende in der Awards-Liste des Debriefings vor den Original-Auszeichnungen und als Chat-Ansage: Kopfzeile, danach je vergebener Auszeichnung Titel, Kriterium, Spieler und Wert.
- **Sprache:** Endpanel und Chat-Zeilen folgen der Spracheinstellung `cl_lang` des jeweiligen Spielers. Deutsch kommt aus `etmain/locale/mod/de.po` im Mod-PK3; alle anderen Sprachen zeigen die englischen Texte. Der Server sendet dafür nur Award-Index, Spieler und Wert (`etlabawards`, `etlabaward`); der Client setzt die Zeile zusammen.
- **Vergabe:** Nur Spieler in Axis oder Allies mit mindestens einem Punkt. Gleichstand geht an den höher platzierten Spieler; Krampus entscheidet Gleichstand wie das Original über Team-Schaden. Gezählt wird nur während der laufenden Runde; die Zähler leben und werden zurückgesetzt wie die Original-Sessionstatistik.
- **Invarianten:** Original-Auszeichnungen, deren Bedingungen und Reihenfolge in `CS_ENDGAME_STATS` bleiben unverändert; die neuen Werte werden angehängt. Der Original-Award „I Ain't Got No Friends“ bleibt mit seiner Schwelle von fünf Teamkills bestehen.

| Titel (en) | Titel (de) | Kriterium |
|---|---|---|
| Christmas Angel | Christkind | Wiederbelebungen; wie beim Revive-XP ohne zuvor im Team getötete Spieler |
| Santa's Little Helper | Weihnachtswichtel | Entschärftes gegnerisches Dynamit und gegnerische Landminen, alle Klassen |
| Star of Bethlehem | Stern von Bethlehem | Per Fernglas entdeckte gegnerische Landminen |
| Krampus | Krampus | Teamkills, ab einem |
| Grinch | Grinch | Zerstörte Bauwerke und Missionsziele, für die das Spiel Zerstörungs-XP vergibt; einfache zerbrechliche Objekte ohne XP-Bonus zählen nicht |
| New Year's Firecracker | Silvesterknaller | Verborgene gegnerische Landminen, durch eigene Handgranate gesprengt |
| Nutcracker | Nussknacker | Kopfschusstreffer aller Waffen |
| Silent Night | Stille Nacht | Messer-Kills, beide Messer |
| Workshop Elf | Werkstatt-Elf | Fertiggestellte Konstruktionen (Endstufe) und vollständig reparierte MG 42 |
| Secret Santa | Geheimwichtel | Von Teamkameraden aufgenommene Health- und Ammo-Packs |

## Nischenwaffen

- **Konfiguration:** `g_etlabWeaponTuning` nennt die aktiven Aufwertungen: `all`, Waffengruppen (`mortar`, `mg`, `satchel`, `flamer`) oder einzelne Namen aus der Tabelle, getrennt durch Leerzeichen, Komma oder Semikolon; unbekannte Namen werden ignoriert. Leer (Standard) entspricht dem bisherigen Verhalten. Wirksam ab dem nächsten Mapstart oder `map_restart`; die Server-Konsole listet die aktiven Aufwertungen.
- **Client und Server:** Der Server sendet die Auswahl als Bitmaske im Konfigstring `CS_ETLAB_WEAPONS`. Server und Client setzen damit dieselben Tabellenwerte; Prediction, Ladebalken, Hitze- und Munitionsanzeige bleiben dadurch synchron.
- **Invarianten:** Beide Teams erhalten dieselben Werte. Primärwaffen, Direkt- und Splash-Schaden von Mörser, MG und Satchel sowie MG-Streuung und Feuerrate bleiben unverändert. Abbauen von Mörser und MG dauert wie bisher. Ausschalten stellt die Originaltabellen bitgenau wieder her. `lowammo` halbiert auch eine aufgewertete Reserve.

| Name | Waffe | Wirkung |
|---|---|---|
| `mortarcharge` | Mörser | Schuss kostet 1/3 statt 1/2 des Soldier-Ladebalkens, ab Heavy Weapons 1 1/4 statt 1/3 |
| `mortarsetup` | Mörser | Aufstellen 1,0 statt 1,667 Sekunden |
| `mortarammo` | Mörser | 20 statt 15 Granaten beim Spawn; Tragekapazität ebenfalls +5 |
| `mortarradius` | Mörser | Explosionsradius 450 statt 400 |
| `mgheat` | Mobiles MG 42/Browning | Überhitzung nach etwa 50 statt 30 Schuss Dauerfeuer (Hitzegrenze 2500 statt 1500) |
| `mgsetup` | Mobiles MG 42/Browning | Aufstellen 0,8 statt 1,25 Sekunden |
| `mgbelt` | Mobiles MG 42/Browning | Zweiter Gurt: 150 Schuss Reserve beim Spawn statt keiner |
| `satchelcharge` | Satchel | Kostet 3/4 statt des vollen Covert-Ops-Ladebalkens, ab Covert Ops 2 1/2 statt 2/3 |
| `satchelradius` | Satchel | Explosionsradius 300 statt 250 |
| `satchelfuse` | Satchel | Zünder löst nach 400 statt 722 ms aus |
| `flamefuel` | Flammenwerfer | 300 statt 200 Treibstoff, also 15 statt 10 Sekunden Dauerfeuer; ein Munitionspaket füllt den Tank |
| `flamespeed` | Flammenwerfer | Tempofaktor 0,85 statt 0,7; gilt wie bisher beim Feuern und ohne Heavy Weapons 3 auch beim Tragen |
| `flamedamage` | Flammenwerfer | 6 statt 5 Schaden je Treffer und Nachbrenn-Tick |

## Übungsfeld-Invarianten

- `g_etlabPractice 1` ist opt-in. Zusätzlich müssen lokaler Host, Listenserver und Map `oasis` zutreffen. Auf Dedicated Servern keine Übungsobjekte erzeugen.
- Bei regulärem Spawn: drei gegnerische scharfe Minen (zwei verborgen, eine markiert) und ein gegnerisches scharfes Dynamitpaket. Blick auf erste verborgene Mine setzen. Revive darf das Feld nicht zurücksetzen.
- Dynamit im Übungsfeld: fünf Minuten Fuse, anschließend echte Explosion; keinem Missionsziel zugeordnet.
- `etlab_targets` / F6 entfernt nur eigene Fixtures und Kartenmarker und erzeugt sie am aktuellen Standort neu. Teams nach Teamwechsel neu bestimmen.
- Bodenpositionen müssen trocken, sichtbar und ausreichend getrennt sein. Bei unzureichendem Platz Meldung statt defekter Fixtures erzeugen.
- `g_etlabPractice 0` stoppt neues Erzeugen; Mapwechsel entfernt bestehende Entitäten.

## Beobachtbare Interaktionen

| Aktion | Kontrollpfad |
|---|---|
| Team/Klasse/Loadout | Limbo, Standardtaste L |
| Akimbo / Primärwaffe / Handgranate | Weapon banks 2 / 3 / 4 |
| Zange | Weapon bank 5; bei mehreren Werkzeugen weiterwechseln |
| Fernglas | Standardtaste B halten; normaler Fortschritt, etwa 2,5 Sekunden |
| Übungsobjekte zurücksetzen | F6 / `etlab_targets` |
| Rundenmodifikator wählen | `callvote modifier <name>`; Liste per `callvote modifier ?` |
| Rundenauszeichnungen | Rundenende: Debriefing, Awards-Liste; Chat |

Native Klassen-/Loadout-Prüfungen und echte Interaktionen getrennt testen. Für Entschärfungszeiten Soldier/Engineer bei identischem Skill 0 und 4 vergleichen; Waffenwechsel und Annäherung nicht mitzählen. Dynamit kann nach Defuse verzögert entfernt werden, daher Frame-Toleranz zulassen. Bestehende XP-/Charge-Regeln nicht unbeabsichtigt ändern.

Technische Einstiege: `code-landkarte.md`. Testvertrag: `pruefstand.md`. Nur auf der eigenen Host-Plattform testen; Cross-platform-Abnahme gehört zum Release.
