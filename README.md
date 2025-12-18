# 2048 — Qt Widgets (C++) with AI Training

Classic 2048 game implemented in C++ using Qt Widgets, featuring
both manual gameplay and an AI training mode based on population-based learning.

The project follows a clean MVC-style architecture:
- game logic implemented in a standalone model
- rendering handled by a custom QWidget
- user input managed by the main window

In addition to manual play, the application includes an AI mode where
an agent learns to play the game through population-based training
(genetic algorithm–style evolution).

---

## Features
- Qt Widgets GUI
- Object-oriented game model (Game2048)
- Clear separation between model, view, and controller
- Manual gameplay mode
- AI training mode with population-based learning
- Score tracking
- Win (2048) prompt with option to continue
- Game-over detection
- Keyboard controls: arrow keys / W, A, S, D
- Clean build artifacts ignored via .gitignore

---

## AI Training Mode

The project includes an optional AI mode where the game is played automatically
by an agent trained using population-based techniques inspired by
genetic algorithms.

### AI Approach
- Each agent evaluates the board state using heuristic features
- A population of agents is trained over multiple generations
- Agents are evaluated based on score and game progress
- Better-performing agents are selected and evolved for the next generation

This mode demonstrates:
- algorithmic thinking
- decision-making under uncertainty
- population-based optimization
- practical application of AI concepts in C++

---

## Controls
- Manual play: Arrow keys or W / A / S / D
- Restart game: "New Game" button
- Enable AI training mode via the application interface

---

## Project Layout
.
├── Game2048.h / Game2048.cpp
├── BoardWidget.h / BoardWidget.cpp
├── MainWindow.h / MainWindow.cpp
├── PopulationWindow.h / PopulationWindow.cpp
├── ai2048.h / ai2048.cpp
├── main.cpp
├── game-2048.pro
└── .gitignore

---

## Technologies
- C++
- STL
- Qt Widgets
- Object-Oriented Programming (OOP)
- Genetic Algorithms (population-based training)
- Heuristic evaluation

---

## Build

### Qt Creator
1. Open game-2048.pro
2. Select a Qt 5 or Qt 6 kit
3. Build and run the project

### Terminal (qmake)

#### Qt 5
sudo apt install qtbase5-dev qt5-qmake qtbase5-dev-tools -y
qmake game-2048.pro
make -j$(nproc)
./game-2048

#### Qt 6
sudo apt install qt6-base-dev qt6-tools-dev-tools -y
qmake6 game-2048.pro
make -j$(nproc)
./game-2048

---

## Purpose
This project was created to practice:
- clean C++ architecture
- object-oriented design
- GUI development with Qt
- algorithmic problem solving
- AI training using population-based methods
- separation of concerns in real-world applications
