# AiloEMU 2.2.0 — compilazione

L'EXE e le DLL nella cartella superiore sono gia' compilati. Nessuno degli
strumenti descritti qui e' necessario per usarli.

## Windows: MSYS2 MinGW64

Servono GCC/MinGW-w64 x86-64 C/C++, windres, GNU Make e CMake. Dalla shell
MSYS2 MinGW64, nella cartella `source`:

```sh
bash build-windows.sh
```

Lo script ricompila il frontend e i tre nuclei storici FCEUmm, Snes9x e
mGBA. Gli undici nuclei aggiunti in 2.0 sono conservati, con i relativi
submodule, in `vendor-sources/additional-cores-source.tar.xz`; estrailo in
una cartella di lavoro e usa i Makefile libretro di ciascun progetto.
Non vengono mai scaricate ROM.

## Cross-compilazione Linux → Windows

Toolchain usata: GCC MinGW-w64 13.2.0 POSIX, header/runtime 11.0.1.
C++17 per il frontend; Make per FCEUmm/Snes9x; CMake per mGBA.

```sh
CC=/usr/bin/x86_64-w64-mingw32-gcc-posix \
CXX=/usr/bin/x86_64-w64-mingw32-g++-posix \
WINDRES=/usr/bin/x86_64-w64-mingw32-windres \
CMAKE=cmake bash build-windows.sh
```

L'uscita audio usa waveOut a 48 kHz; `resampler.hpp` converte in streaming
le frequenze dichiarate dai nuclei. Video GDI: 4:3 NES/SNES, 10:9 GB/GBC, 3:2 GBA.
La selezione della console usa estensioni case-insensitive e un controllo
preliminare dell'intestazione/dimensione, seguito dal loader del nucleo.
I nuclei vengono scaricati e inizializzati in sequenza, mai eseguiti insieme.

## Revisioni e configurazioni

- FCEUmm: `236ccdfc911e84c60fea6b9d0699c2d440a8de14`, HAVE_HDPACK=0.
- Snes9x: `890b5d445538fe790aa3add3d5702c80f551e0ae`, LTO disattivato,
  runtime collegato staticamente. Nessuna modifica al nucleo.
- mGBA: `7a12d6d4b9acb14c0ae62c9166b6a2f3d08007f6`, solo target libretro,
  frontend QT/SDL e dipendenze esterne disattivati; runtime statico.
  Il frontend seleziona BIOS HLE, senza BIOS Nintendo.

Nuclei AiloEMU 2.0:

- Genesis Plus GX `c2838c7dc423` — Mega Drive, Master System, Game Gear, SG-1000.
- PicoDrive `ab021146b70e` — Sega 32X; CD disabilitato nell'interfaccia.
- Stella 2014 `7d1361e407e6` — Atari 2600.
- ProSystem `8a88014287c7` — Atari 7800.
- Beetle NeoPop `a50d5ac288a8` — Neo Geo Pocket/Color.
- Beetle WonderSwan `4b01295838ea` — WonderSwan/Color.
- Beetle PCE Fast `eed8075ece35` — PC Engine; contenuti CD non esposti.
- DeSmuME 2015 `422b688009cc` — renderer software, firmware/BIOS interno.
- ParaLLEl N64 `2f3bf60dcd96` — Angrylion software, senza OpenGL/Vulkan/dynarec.
- Beetle VB `83ed42608601` — Virtual Boy.
- PokeMini `132111b76343` — Pokémon Mini.

Per i Makefile classici la cross-compilazione usata segue questo schema:

```sh
make -f Makefile platform=win CC=x86_64-w64-mingw32-gcc-posix \
  CXX=x86_64-w64-mingw32-g++-posix AR=x86_64-w64-mingw32-ar -j4
```

Genesis Plus GX e PicoDrive usano `Makefile.libretro`; PokeMini usa
`platform=windows_x86_64`; DeSmuME si compila dalla directory `desmume`.
ParaLLEl N64 è stato costruito con:

```sh
make platform=win HAVE_OPENGL=0 HAVE_PARALLEL=0 HAVE_GLIDE64=0 \
  HAVE_GLIDEN64=0 HAVE_GLN64=0 HAVE_RICE=0 WITH_DYNAREC= \
  CC=x86_64-w64-mingw32-gcc-posix CXX=x86_64-w64-mingw32-g++-posix \
  AR=x86_64-w64-mingw32-ar -j4
```

I sorgenti storici sono inclusi in `vendor/`; quelli 2.0 nell'archivio sopra.
Sono esclusi metadati Git,
object/binary intermedi e la cartella di fixture grafiche `mgba/cinema`,
che non serve alla compilazione del core.
Senza metadati Git, mGBA puo' riportare `0.11.0` anziche' la stringa della
build distribuita. Gli stati rapidi sono vincolati alla versione del core;
non e' garantita la compatibilita' degli stati con una ricompilazione.
I salvataggi batteria rimangono dati grezzi del gioco.

## Test e demo

`../AiloEMU.exe --self-test` verifica le cinque console dotate di demo attraverso le
DLL Windows e scrive `../test-results/report.txt`. Non apre l'interfaccia
ne' il dispositivo audio. I test non certificano altoparlanti o controller.

Per testare su Linux, compilare copie separate dei core per Unix, quindi:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror test_runner.cpp -ldl -o test_runner
./test_runner path/fceumm_libretro.so ../roms/Ailo-Demo.nes test-output

g++ -std=c++17 -O2 -Wall -Wextra -Werror multisystem_runner.cpp -ldl -o multisystem_runner
./multisystem_runner path/snes9x_libretro.so path/mgba_libretro.so ../roms test-output
```

Il caricamento Nintendo DS e la pulizia della cache possono essere verificati
senza ROM commerciali con `nds_smoke_runner.cpp`, che genera in memoria una
piccola immagine homebrew di test:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror nds_smoke_runner.cpp -ldl -o nds-smoke
./nds-smoke path/desmume2015_libretro.so test-output-nds
```

Per Snes9x: `make -C vendor/snes9x/libretro platform=unix LTO= -j4`
in una copia senza object Windows. Per mGBA usare le opzioni CMake dello
script, omettendo CMAKE_SYSTEM_NAME/compilatori Windows e linker statico.

Le demo sono originali e dedicate al pubblico dominio (CC0).
Per rigenerarle, Python 3; solo il generatore GBA richiede Keystone:

```sh
python3 make_demo.py
python3 -m pip install keystone-engine==0.9.2
python3 make_extra_demos.py
```

`make_extra_demos.py` incorpora piccoli assembler 65c816/Game Boy e usa Keystone
per il codice ARM. `gba_demo.s` e' la copia leggibile dell'assembly GBA
prodotta dal generatore. Grafica originale; le demo GB/GBC contengono soltanto i quattro byte di firma richiesti dal riconoscimento mGBA, non il logo completo.
La demo GBA e' destinata all'avvio tramite HLE senza verifica del logo BIOS.

## File principali

- `main.cpp`: UI, menu, selezione core, input, pacing, audio e video Windows.
- `settings_ui.hpp`: controller pixel art interattivi, associazioni tastiera/gamepad e temi colore.
- `core.hpp`: adapter libretro, caricamento ROM, framebuffer e salvataggi.
- `resampler.hpp`: conversione audio stereo a 48 kHz.
- `diagnostics.hpp`, `multisystem_tests.hpp`: test di integrazione.
- `nds_smoke_runner.cpp`: test di caricamento e cache Nintendo DS.
- `make_demo.py`, `make_extra_demos.py`: generatori delle cinque demo.
- `build-windows.sh`: compilazione di tutti i componenti.
- `vendor/`: sorgenti FCEUmm, Snes9x e mGBA.
- `vendor-sources/additional-cores-source.tar.xz`: sorgenti degli altri nuclei.

Il frontend e' GPL-2.0-or-later con l'eccezione di collegamento descritta
in `../licenses/AiloEMU-Core-Linking-Exception.txt`. Le licenze dei nuclei
restano quelle dei rispettivi autori, inclusi i limiti non commerciali di Snes9x.
