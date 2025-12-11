# 2048 — Qt Widgets (C++)

Classic 2048 game implemented in C++ using Qt Widgets.  
The project uses a simple MVC-style split: game logic in a standalone model, rendering in a QWidget, input handled by the main window.

## Features
- Qt Widgets GUI
- OOP game model (`Game2048`)
- Score tracking
- Win (2048) prompt with option to continue
- Game-over detection
- Keyboard controls: arrows / WASD
- Clean build artifacts ignored via `.gitignore`

## Controls
- **Move:** Arrow keys or **W/A/S/D**
- **Restart:** “New Game” button

## Project Layout
.
├── Game2048.h / Game2048.cpp # Game model (state + rules)
├── BoardWidget.h / BoardWidget.cpp # Board rendering
├── MainWindow.h / MainWindow.cpp # UI + input controller
├── main.cpp # App entry point
├── game-2048.pro # qmake project
└── .gitignore

## Build

### Qt Creator
1. Open `game-2048.pro`
2. Select a kit (Qt 5 or Qt 6)
3. Build & Run

### Terminal (qmake)
Qt5:
```bash
sudo apt install qtbase5-dev qt5-qmake qtbase5-dev-tools -y
qmake game-2048.pro
make -j$(nproc)
./game-2048
```
Qt6 (if installed):
```bash
sudo apt install qt6-base-dev qt6-tools-dev-tools -y
qmake6 game-2048.pro
make -j$(nproc)
./game-2048
```
