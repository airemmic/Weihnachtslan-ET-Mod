---
name: etlab-mod
description: Entwickle den Weihnachtslan-ET-Gamecode-Mod, sichere Aenderungen als Patch und pruefe ausschliesslich auf der jeweiligen Host-Plattform; Cross-platform-Abnahme beim beauftragten Release.
---

# Weihnachtslan-ET-Mod bearbeiten

Projektwurzel ist drei Ebenen oberhalb dieses Skill-Ordners. `AGENTS.md` und `README.md` lesen. Dokumentation ist fuer LLM-Agenten; nur die README-Einleitung richtet sich an Menschen.

## Plattformentscheidung zuerst

Windows-Arbeitsrechner: nur Windows bauen/testen. Linux-Arbeitsrechner: nur Linux bauen/testen. Keine zusaetzliche Gegenplattform via WSL, Container, VM oder SSH starten. Cross-platform-Abnahme nur im ausdruecklich beauftragten Release; vorhandene Infrastruktur allein autorisiert sie nicht.

## Workflow

- Windows: `scripts/Build-Mod.ps1`; Source-State `.cache/state.json`; danach `scripts/Save-ModChanges.ps1`.
- Linux: `python3 scripts/etlab-linux.py build`; Source-State `.cache/linux/state.json`; danach `python3 scripts/etlab-linux.py save`.
- Source-Pfad aus State lesen. Gamecode dort bearbeiten; kanonische Aenderungen nach `mod/etlab.patch` sichern. Neue Source-Dateien mit `git add -N` erfassen. Tests direkt in `tests/native/` pflegen.
- Nur Mod-Module bauen. Engine aus offiziellem Stable-Archiv. Normale Builds pruefen Latest; `-Locked`/`--locked` bewusst fuer reproduzierbare Pruefungen verwenden. Konflikte/ungesicherte Edits nicht automatisch entfernen.
- Details: `docs/llm-workflow.md`; Linux-CLI: `docs/linux.md`; Installation/Release: `docs/installation.md`.

## Gameplay-Fallstricke

Vor Aenderungen `docs/spielregeln.md` und passende Symbole aus `docs/code-landkarte.md` lesen.

- Spawn, Limbo und Servervalidierung muessen Akimbo gemeinsam erlauben; beide Magazine, XP-Freischaltung und Klassenwechsel pruefen.
- Zangenbesitz und Ausfuehrungsberechtigung getrennt pruefen: `G_CanUsePliersOn`, `Weapon_Engineer`, Interaktionshinweise. `PM_Weapon` ist gemeinsame Prediction/Server-Logik; 100 statt 50 ms nur fuer berechtigte Nicht-Engineers.
- Minensuche umfasst Fernglas, `G_CheckSpottedLandMines`, Snapshot-Sichtbarkeit und Hinweise. `G_RevealLandmine` erzeugt den beschiessbaren Trefferbereich. Klassen-Ausnahmen aus Gameplay-Vertrag erhalten.
- Verborgene Minen akzeptieren nur beide Handgranaten-Schadensarten; `takedamage` allein reicht nicht. Radius/Deckung und scharfer Zustand bleiben Bedingungen.
- Uebungsfeld: lokaler Host + Listenserver + Oasis + Opt-in. Vector-Makros werten Argumente mehrfach aus; keine Inkremente in Makroargumenten.

## Nachweise

Build/CTest vor Staging; danach nur aufgabenrelevante Tests der Host-Plattform. `docs/pruefstand.md` beschreibt Aussagekraft und Testgrenzen. Reine Doku braucht keinen Spielstart. Granaten werden im Client-Test positioniert, die Explosion bleibt echtes Gameplay. Native Assertions ersetzen keine Client-/Multiplayer-Abnahme.

Gemischte LAN-Pakete: identisches PK3 mit beiden Clientmodulen; Export prueft Commit/Patch. ZIP-Strukturpruefung von echter Cross-platform-Verbindung unterscheiden. Letztere erst beim beauftragten Release.
