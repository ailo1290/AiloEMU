# AiloEmu

Emulatore multiconsole per Windows e Linux/Bazzite.

## Versioni conservate

- `windows/`: sorgenti originali della versione Windows **2.2.4**.
- `linux/`: sorgenti originali della versione Linux/Bazzite **2.3.0**.

Le due versioni sono conservate separatamente: il file `core.hpp` differisce.
Gli altri sorgenti condivisi coincidono; Linux aggiunge il frontend SDL e il suo script di compilazione.

## Contenuto

Ogni cartella contiene i sorgenti del frontend, i test esistenti, gli script di compilazione,
i sorgenti dei core in `source/vendor/` e `source/vendor-sources/`, le licenze e la documentazione originale.
Sono incluse soltanto le demo originali distribuite con il progetto. Non sono inclusi giochi commerciali, BIOS o salvataggi personali.

## Download del programma

Pubblicare nelle GitHub Releases i pacchetti originali:

- `AiloEMU-Windows-x64.zip` (Windows 2.2.4)
- `AiloEMU-Bazzite-x86_64-2.3.0.tar.gz` (Linux 2.3.0)

Gli eseguibili e le librerie precompilate rimangono nei pacchetti delle Releases.
Questa repository contiene i sorgenti; gli installer e gli script di verifica della distribuzione richiedono i relativi file compilati.

## Compilazione

Per Windows consultare `windows/source/BUILD.md` ed eseguire `build-windows.sh` dalla shell MSYS2/MinGW64.
Lo script compila il frontend e i tre core di base; gli altri core richiedono le istruzioni del documento.

Per Linux consultare `linux/source/build-linux.sh`: lo script compila soltanto il frontend,
richiede GCC C++17 e SDL2 e non compila automaticamente i core. I core devono essere compilati separatamente
dai sorgenti inclusi e collocati in `linux/cores/`.

La documentazione originale può riportare numeri di versioni precedenti per i componenti storici.
Questa preparazione della repository non modifica il codice e non include una nuova compilazione o una nuova verifica funzionale.

## Flatpak / Flathub

`linux/flathub/` contiene il manifest di prova originale, non ancora pronto per un invio a Flathub.
Leggere il relativo README: occorrono sorgenti pubblici verificabili, la compilazione dei core e un App ID associato a un account controllato.

## Licenze

Il frontend è distribuito sotto **GPL-2.0-or-later con l'eccezione di collegamento inclusa**.
Consultare `LICENSE`, `LICENSE-EXCEPTION.txt` e le cartelle delle licenze di ciascuna piattaforma.
I core conservano le proprie licenze, incluse le condizioni non commerciali documentate per alcuni componenti.
Non applicare una licenza MIT a tutto il progetto.

## Caricamento su GitHub

Estrarre questo ZIP e caricare il suo contenuto nella radice della repository.
Poiché contiene migliaia di file, per il primo caricamento è preferibile usare un client Git.
Lo ZIP della repository non va caricato al posto dei singoli file sorgente.
