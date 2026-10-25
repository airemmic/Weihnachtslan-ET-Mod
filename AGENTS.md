# ETLab-Mod

Dieses Verzeichnis ist das eigenständige Mod-Repository. Diese Regeln konkretisieren den Arbeitsablauf für diesen Ordner.

- README.md und `.agents/skills/etlab-mod/SKILL.md` lesen. Freigegebene Gameplay-Regeln stehen in `docs/spielregeln.md`.
- Nur Gamecode-Mod entwickeln. Original-Engine aus offiziellem Stable-Archiv benutzen; externe Libraries nicht ändern.
- Standard: `scripts/Build-Mod.ps1` ermittelt den neuesten veröffentlichten Stable-Release. `-Locked` nur für einen bewusst reproduzierbaren Teststand verwenden.
- Aktiver Source-Pfad steht in `.cache/state.json`. Gamecode dort bearbeiten, danach `scripts/Save-ModChanges.ps1` ausführen, damit `mod/etlab.patch` die Änderung enthält. Neue Source-Dateien mit `git add -N` erfassen. Keine eigenen Commits im Abhängigkeits-Checkout.
- Native Tests ausschließlich in `tests/native/` bearbeiten; Prepare kopiert sie in den Build-Checkout. Keine ungesicherten Source-Änderungen überschreiben oder durch Reset entfernen.
- Regulär mit Build-Mod bauen; CTest läuft vor dem Staging. Runtime-Tests belegen andere Eigenschaften als native Tests. Echte Client-Tests schließen ihren eigenen Spielprozess; Originalprofile bleiben unberührt.
- Windows PowerShell 5.1, UTF-8 mit BOM und CRLF für PS1. Markdown mit YAML-Frontmatter ohne BOM. Vorhandene Source-Encodings und Zeilenenden erhalten.
- Lokale Assets, Downloads, Builds, Profile und Logs nicht committen. Kein GitHub-Push und keine Server-Umstellung ohne Auftrag.
