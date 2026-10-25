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
- **Taktische Modi, ganze Map:** `doublexp` doppelte XP; `fastspawn` Verstärkungszeit halbiert (Server und Client-Timer); `longdyno` Dynamit 45 statt 30 Sekunden; `quickspot` Fernglas-Minensuche doppelt so schnell; `medicshortage` höchstens zwei Medics je Team, Medic-Ladezeit ×1,5; `engineerfest` Minenlimit verdoppelt, Engineer-Ladezeit halbiert; `heavyfree` keine Teamlimits für schwere Waffen; `lowammo` Reservemunition beim Spawn halbiert, Handgranaten und Magazin unverändert; `suddendeath` keine Verstärkung in den letzten zwei Minuten eines Timelimits; `classrotation` wer mit derselben Klasse wie beim letzten Spawn erneut spawnt, erhält die nächste Klasse mit deren Standard-Loadout.
- **Gaudi-Modi, zeitlich begrenzt ab Matchstart:** `lowgravity` halbe Schwerkraft; `sidearms` keine Primärwaffen und Handgranaten, Klassenwerkzeuge inklusive Dynamit und Minen bleiben; `iceskating` Laufgeschwindigkeit ×1,4, Rückstoß verdoppelt; `grenaderain` Handgranaten alle 20 Sekunden aufgefüllt; `headshots` Spieler verletzen Spieler nur per Kopfschuss, Rückstoß bleibt; `vampire` jeder Gegner-Kill heilt 20 Lebenspunkte bis zum Maximum.
- **Invarianten:** Ohne aktiven Modus entsprechen alle Werte dem bisherigen Verhalten. Ladezeiten werden samt Konfigstring neu gesetzt; Map-Script-Faktoren bleiben multiplikativ erhalten. Nicht-scharfe Werte wie das Minenlimit `-1` bleiben unverändert.

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

Native Klassen-/Loadout-Prüfungen und echte Interaktionen getrennt testen. Für Entschärfungszeiten Soldier/Engineer bei identischem Skill 0 und 4 vergleichen; Waffenwechsel und Annäherung nicht mitzählen. Dynamit kann nach Defuse verzögert entfernt werden, daher Frame-Toleranz zulassen. Bestehende XP-/Charge-Regeln nicht unbeabsichtigt ändern.

Technische Einstiege: `code-landkarte.md`. Testvertrag: `pruefstand.md`. Nur auf der eigenen Host-Plattform testen; Cross-platform-Abnahme gehört zum Release.
