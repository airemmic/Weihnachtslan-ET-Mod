---
name: etlab-mod
description: Entwickle den eigenstaendigen ETLab-Gamecode-Mod gegen den neuesten offiziellen ET-Legacy-Stable-Release und pruefe Build, Minenregeln und Entschaerfung.
---

# ETLab-Mod bearbeiten

Projektwurzel ist drei Ebenen oberhalb dieses Skill-Ordners. Lies `README.md` fuer den Workflow, `docs/spielregeln.md` vor Gameplay-Aenderungen und `docs/code-landkarte.md` fuer Symbole.

Der versionierte Mod besteht aus `mod/etlab.patch`, eigenen Tests und Werkzeugen. `.cache/state.json` nennt den aktuellen vorbereiteten Source-Ordner. Dort Gamecode bearbeiten, anschliessend `scripts/Save-ModChanges.ps1` ausfuehren. Native Tests direkt in `tests/native/` pflegen. Sonst gingen Aenderungen beim Teilen des Mod-Repositorys verloren.

Mit `scripts/Build-Mod.ps1` bauen; nur Game-Module und Test-Executable werden kompiliert. Der normale Starter prueft den neuesten Stable-Release. Kein Wechsel auf master/Nightly. Konflikte beim Update nachvollziehen und Patch auf neuer Release-Basis anpassen, niemals ungesicherte Aenderungen automatisch entfernen. Fuer Wiederholungspruefungen bewusst `-Locked` nutzen.

## Gameplay-Fallstricke

- Akimbo-Regeln fuer Spawn und Limbo gemeinsam pflegen; beide Magazine und XP-Freischaltung erhalten.
- Zangenrechte: `G_CanUsePliersOn` und `Weapon_Engineer`. Nicht-Engineers nur scharfes Dynamit und Minen. `PM_Weapon` setzt den gemeinsamen Client-/Server-Takt auf 100 statt 50 ms; gleiche Skill-Boni erhalten.
- Minensuche braucht Fernglas, `G_CheckSpottedLandMines`, Snapshot-Sichtbarkeit und Hinweise. `G_RevealLandmine` erzeugt den beschiessbaren Trefferbereich.
- Verborgene Minen akzeptieren ausschliesslich beide Handgranaten-Schadensarten. `takedamage` allein reicht nicht; Radius/Deckung gelten weiterhin.
- Uebungsobjekte nur fuer lokalen Listenserver, Oasis und `g_etlabPractice`. Native Vector-Makros werten Argumente mehrfach aus; keine Inkremente in Makroargumenten.

## Nachweis

Build fuehrt CTest vor Staging aus. Danach `Test-ETLabServer.ps1` fuer offiziellen Server und `Start-ETLabGame.ps1 -MineSmoke` fuer echten Client verwenden. Der Client-Test dauert etwa zwei Minuten, nutzt ein frisches Profil und vergleicht Soldier/Engineer bei Skill 0/4. Granaten werden fuer reproduzierbare Positionierung testseitig versetzt, ihre Explosion ist echtes Gameplay. Screenshot ansehen; Logs liegen unter `logs/` und `profiles/`.

Native Assertions, Runtime-Nachweise und manuelle Multiplayer-Abnahme getrennt berichten. Nur Windows x64 ist automatisiert; DLLs nicht als Linux-Server-Paket ausgeben.
