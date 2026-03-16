# WxCam

Application desktop C++/wxWidgets avec flux vidéo en temps réel.

## Fonctionnalités

| Bouton | Action |
|--------|--------|
| **Screenshot** | Capture la frame courante → PNG horodaté dans `screenshots/` |
| **Archive** | Crée un ZIP de tous les PNG → dialogue de sauvegarde |
| **Purge** | Supprime tous les PNG du dossier `screenshots/` |
| **Fetch posts** | Récupère les 20 derniers posts Bluesky → fichier texte |

## Dépendances
 
- **wxWidgets 
- **OpenCV 
- **CMake 
---

### Linux (Ubuntu/Debian)

```bash
# Dépendances système
sudo apt update
sudo apt install -y \
    build-essential cmake \
    libwxgtk3.2-dev \
    libopencv-dev

# Clone the repository:

git clone https://github.com/BENAISSA-Tayeb/wxcam

cd wxcam
# Build
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)

# Lancer
./WxCam
```


## Architecture

```
wxcam/
├── CMakeLists.txt
├── include/
│   ├── App.h           # wxApp entry point
│   ├── CameraPanel.h   # Panel caméra
│   └── MainFrame.h     # Fenêtre principale
└── src/
    ├── App.cpp
    ├── CameraPanel.cpp  # Thread capture + rendu OpenCV→wxBitmap
    └── MainFrame.cpp    # Logique des 4 boutons
```
