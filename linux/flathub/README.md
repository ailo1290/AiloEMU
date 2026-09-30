# Stato del manifest Flathub

Il manifest incluso è utilizzabile per un test locale spostandolo nella radice di `AiloEMU-Linux` e lanciando `flatpak-builder`. Non è ancora inviabile a Flathub perché `type: dir` e i core precompilati non rispettano il modello di build pubblica.

Per la candidatura pubblica occorre:

1. scegliere un App ID basato su un dominio o account GitHub realmente controllato;
2. pubblicare i sorgenti AiloEMU con un tag firmato/stabile;
3. sostituire `type: dir` con sorgenti HTTPS dotati di SHA-256;
4. compilare ogni core libretro da sorgente nel manifest;
5. aggiungere almeno uno screenshot 16:9 senza ROM commerciali;
6. aprire la pull request al repository `flathub/flathub` e superare la revisione.

Non creare il namespace `io.github.ailoemu` se non si controlla l'omonimo account o organizzazione GitHub.
