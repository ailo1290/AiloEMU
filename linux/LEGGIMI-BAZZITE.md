# AiloEMU 2.3.0 per Bazzite/Linux x86_64

Questa cartella contiene una build Linux nativa. Non usa Wine e non contiene ROM o BIOS.

## Installazione locale su Bazzite

1. Estrai l'archivio senza spostare singolarmente i file.
2. Apri un terminale nella cartella estratta.
3. Esegui `chmod +x install-bazzite.sh && ./install-bazzite.sh`.
4. Avvia **AiloEMU** da Bazaar/menu applicazioni oppure aggiungilo a Steam come gioco non-Steam.

Le ROM possono rimanere in `~/Documenti/ROMS`. Il browser interno mostra cartelle e ROM supportate. Usa Invio/A per aprire, Backspace/B per tornare indietro, Ctrl+O per riaprire il browser e F11 per lo schermo intero.

## Comandi predefiniti

- Tastiera: frecce, Z=B, X=A, A=Y, S=X, Invio=Start, Maiusc=Select, Ctrl=L.
- Gamepad SDL/XInput/Steam Input: mappatura standard; fino a quattro controller sono assegnati in ordine ai giocatori 1-4.
- F5 salva lo stato nello slot 1; F9 lo carica; Spazio mette in pausa.

## Sistemi

NES, SNES, GB, GBC, GBA, Nintendo DS, Nintendo 64, Mega Drive, Master System, Game Gear, SG-1000, Sega 32X, Atari 2600/7800, Neo Geo Pocket/Color, WonderSwan/Color, PC Engine, Virtual Boy e Pokemon Mini.

## Pubblicazione in Bazaar

Bazaar usa il catalogo Flathub. La cartella `flathub/` contiene il punto di partenza del manifest; prima dell'invio pubblico occorrono un repository sorgente pubblico, un namespace App ID controllato dallo sviluppatore, screenshot e la compilazione dei core dai rispettivi sorgenti. Il pacchetto locale serve per testare subito AiloEMU su Bazzite, ma non sostituisce la revisione Flathub.
