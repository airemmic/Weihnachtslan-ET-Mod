# Fertigen Mod installieren

Dieses Paket ist für Windows x64. ET: Legacy muss installiert sein; Engine und Original-Spieldaten sind nicht im Mod-Paket. Die konkret gebaute ET-Version steht in `manifest.json`.

1. Den Ordner `etlab` aus dem ZIP neben `etmain` in die ET: Legacy-Installation kopieren. Für saubere Updates einen vorhandenen eigenen `etlab`-Ordner vorher separat sichern und vollständig durch den neuen ersetzen; keine alten PK3s mischen.
2. Lokal starten: `etl.exe +set fs_game etlab +set g_etlabPractice 1 +map oasis`.
3. Dedicated Windows-Server: `etlded.exe +set dedicated 2 +set fs_game etlab +exec server.cfg`. Netzwerk, Maps und Downloadserver nach eurer Serverkonfiguration einrichten. `g_etlabPractice` ist dort nicht aktiv.

Alle Spieler brauchen die passenden Client-Module bzw. das Mod-PK3; der Server braucht das passende `qagame`-Modul. Ein Downloadserver kann das Mod-PK3 an Clients liefern. Dies wird von unseren lokalen Teststartern nicht eingerichtet; sie sind bewusst auf Loopback beschränkt.

Windows-DLLs sind keine Linux-Module. Für einen Linux-Server muss derselbe Mod-Stand für Linux und die Serverarchitektur gebaut werden. Linux-/macOS-Builds und gemischte Multiplayer-Sitzungen sind in diesem Projekt noch nicht abgenommen.

Die gemeinsame Weiterentwicklung erfolgt am Mod-Projekt, nicht an Dateien aus diesem fertigen ZIP. Das separat exportierte Projekt-ZIP enthält Patch, Tests, Buildskripte, Lizenz und Release-Lock. Original-Spieldaten müssen aus einer vorhandenen Installation kommen.
