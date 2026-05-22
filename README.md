# Quoridor Game

A complete implementation of the abstract strategy board game **Quoridor**, built in C++ with a Qt graphical user interface. Developed as a term project for **CSE471s: Artificial Intelligence — Spring 2026**.

---

## Table of Contents

- [Game Description](#game-description)
- [Features](#features)
- [Installation and Running](#installation-and-running)
- [Controls](#controls)
- [Game Modes and AI Difficulty](#game-modes-and-ai-difficulty)
- [Project Structure](#project-structure)
- [Demo Video](#demo-video)
- [Team](#team)

---

## Game Description

Quoridor is a 2–4 player abstract strategy game designed by Mirko Marchesi (Gigamic, 1997). In this implementation you play the 2-player version on a 9×9 board.

**Objective:** Be the first player to move your pawn to any cell on the opposite side of the board.

**Core Rules:**

- Each player starts at the centre of their baseline (Player 1 at row 8, Player 2 at row 0) with **10 walls**.
- On each turn a player must either **move their pawn** or **place a wall** — not both.
- Pawns move one square **orthogonally** (up, down, left, right). Diagonal movement is not normally allowed.
- If your pawn is adjacent to the opponent's and no wall blocks the way, you can **jump straight over** them.
- If the straight jump is blocked by a wall or the board edge, you may instead **move diagonally** around the opponent.
- Walls span **two cell edges** and block movement through those edges permanently.
- A wall **cannot** be placed if it would completely cut off either player's path to their goal.
- First pawn to reach **any cell** of the opposite baseline wins.



## Features

- **Complete Quoridor ruleset** — all movement, jump, diagonal-jump, and wall rules implemented
- **Human vs. Human** — local two-player mode on one machine
- **Human vs. AI** — play against the computer at three difficulty levels
- **Three AI difficulty levels:**
  - *Easy* — follows the BFS shortest path to the goal; never places walls
  - *Medium* — greedy shortest-path + selective wall blocking
  - *Hard* — Minimax search (depth 2) with alpha-beta pruning, capped at 300 ms
- **Valid move highlighting** — togglable overlay shows all legal pawn destinations
- **Wall ghost preview** — semi-transparent wall follows the mouse before you click
- **Wall counter** — always shows how many walls each player has left
- **Turn indicator** and status messages for invalid moves and game-over
- **Undo / Redo** *(bonus feature)* — full game history via the Memento pattern (up to 100 states)
- **Game reset** at any time
- **Keyboard shortcuts** for power users

---

## Installation and Running

### Prerequisites

| Requirement | Minimum Version |
|---|---|
| C++ compiler | C++17 (GCC 9+, Clang 10+, MSVC 2019+) |
| Qt | Qt 5.15 or Qt 6.x |
| CMake | 3.19+ |

### Build with CMake (Recommended)

```bash
# 1. Clone the repository
git clone https://github.com/<your-username>/quoridor-game.git
cd quoridor-game

# 2. Create a build directory
mkdir build && cd build

# 3. Configure — CMake will auto-detect Qt6 or fall back to Qt5
cmake ..

# 4. Build
cmake --build . --config Release

# 5. Run
./Quoridor          # Linux / macOS
Quoridor.exe        # Windows (Release\Quoridor.exe on MSVC)
```

### Build with Qt Creator

1. Open `Quoridor Game/Quoridor.pro` in Qt Creator.
2. Select a Qt Kit (Qt 5.15+ or Qt 6).
3. Click **Build** then **Run**.

### Build with CMake + Qt Creator

1. Open Qt Creator → **File → Open File or Project** → select `CMakeLists.txt`.
2. Choose a kit and click **Configure Project**.
3. Click **Build** and then **Run**.

> **Windows note:** After a Release build, run `windeployqt Quoridor.exe` to copy the required Qt DLLs alongside the executable so it can be distributed.

---

## Controls

| Action | How |
|---|---|
| **Select pawn destination** | Left-click a highlighted cell (in Move Pawn mode) |
| **Place a wall** | Switch to Wall mode, hover to preview, left-click to place |
| **Rotate wall orientation** | Right-click (or press **R**) while in Wall mode |
| **Toggle Move / Wall mode** | Click the mode buttons in the sidebar, or press **W** |
| **Show / hide valid moves** | Click the *Show Valid Moves* toggle button |
| **Undo** | Ctrl+Z or click the **Undo** button |
| **Redo** | Ctrl+Y or click the **Redo** button |
| **Reset game** | Click the **Reset** button |

---

## Game Modes and AI Difficulty

Select the mode and difficulty in the **startup dialog** before the game begins. You can also change the AI difficulty mid-game using the dropdown in the sidebar.

| Mode | Description |
|---|---|
| Human vs. Human | Two people take turns on the same computer |
| Human vs. AI | Play against the computer (Easy / Medium / Hard) |

| Difficulty | Strategy | Wall Usage |
|---|---|---|
| Easy | Always follows BFS shortest path | Never |
| Medium | Greedy BFS + places walls that significantly delay the opponent | Yes |
| Hard | Minimax depth-2 with alpha-beta pruning (300 ms cap) | Yes |

---

## Project Structure

```
Quoridor Game/
├── main.cpp            # Entry point, startup dialog
├── Position.h          # Lightweight (row, col) coordinate struct
├── Wall.h / .cpp       # Wall data model, overlap & cross detection
├── Board.h / .cpp      # 9×9 board state, move validation, BFS
├── Player.h / .cpp     # Player data (position, wall count, goal row)
├── GameState.h         # Immutable game snapshot (Memento pattern)
├── Game.h / .cpp       # Central controller, turn management, undo/redo
├── AIPlayer.h / .cpp   # AI logic — Easy, Medium, Hard
├── BoardWidget.h/.cpp  # Qt widget: renders board, handles mouse input
├── MainWindow.h/.cpp   # Top-level Qt window, sidebar controls
├── GameWindow.h/.cpp   # Alternate window layout
├── CMakeLists.txt      # CMake build configuration (Qt5/Qt6 auto-detect)
└── Quoridor.pro        # Qt Creator .pro file (legacy)
```

**Architecture at a glance:**

```
main.cpp (StartupDialog)
    └── MainWindow
            ├── BoardWidget  ──reads──►  Game  ──delegates──►  Board
            └── Sidebar controls              └── AIPlayer
```

`BoardWidget` never contains game rules — all logic lives in `Game` and `Board`.

---

## Demo Video

> ** https://drive.google.com/drive/folders/1ouFU0PbZJACJvIA_XUjD3nc3pi0F5xI4**

The video covers:
1. Application startup and mode selection
2. Human vs. Human full game
3. Human vs. AI gameplay at each difficulty level
4. Undo/Redo demonstration

---

## Team

| Name | Student ID |
|---|---|
| Safie Tarek Mahmoud | 2300136 |
| Moamen Ahmed Badr | 2300636 |
| Ibrahim Mahrous Ahmed Mohamed | 2300173 |
| Mohamed Emad Mohamed Helal | 2300615 |

**Course:** CSE471s — Artificial Intelligence  
**Semester:** Spring 2026  
**Instructor:** Manal Morad Zaki

