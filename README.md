# Quoridor Game Implementation

> **CSE471s: Artificial Intelligence — Spring 2026 Term Project**
>
> A complete implementation of the abstract strategy board game Quoridor with GUI, AI opponents, and full game mechanics.

---

##  Game Description

**Quoridor** is a strategy board game invented by Mirko Marchesi (1997). Players take turns either moving their pawn toward the opposite side of the board or placing walls to obstruct their opponent. The first player to reach any square on the opposite baseline wins.

### Rules Implemented
- **Board**: 9×9 grid
- **Players**: 2 players (local)
- **Pieces**: Each player has 1 pawn (starts at center of baseline) and 10 walls
- **Objective**: Be the first to move your pawn to the opposite side
- **Movement**: One square orthogonally; jump over adjacent opponent if not blocked; diagonal fallback when jump is blocked by wall
- **Walls**: 2 squares long, placed between cells; cannot overlap, cross, or completely block any player's path to goal

---

##  Features

### Core Requirements
-  Complete 2-player Quoridor ruleset implementation
-  Graphical User Interface (Qt-based)
-  Game state visualization (board, pawns, walls, turn indicator)
-  Valid move highlighting (green cells)
-  Illegal move prevention with visual feedback
-  Path-finding validation for wall placement (BFS ensures no player is trapped)

### Game Modes
- **Human vs Human** — Local two-player mode on the same computer
- **Human vs AI** — Play against computer opponent with selectable difficulty

### User Interface
- Clear board rendering with goal row highlighting
- Intuitive click-to-move / click-to-place-wall controls
- **Gold turn indicator ring** around active player's pawn
- Real-time wall count display per player
- Status messages (current turn, invalid move warnings, winner announcement)
- **Right-click to rotate wall orientation** while placing walls
- Game reset functionality (F2 or New Game button)

### Bonus Features
- **AI Difficulty Levels** (Easy, Medium, Hard)
- **Undo / Redo** — Full game state history with Memento pattern (Ctrl+Z / Ctrl+Y)
- AI opponent with three distinct algorithms (see AI section below)

---


##  Installation & Running

### Requirements
- Qt 5.15+ or Qt 6.x
- C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.19+ **or** qmake

### Build with CMake (Recommended)

```bash
git clone <your-repo-url>
cd quoridor
cmake -B build -S . -DCMAKE_PREFIX_PATH="/path/to/Qt/6.x.x/gcc_64"
cmake --build build --config Release
./build/Quoridor
```

### Build with qmake

```bash
git clone <your-repo-url>
cd quoridor
qmake Quoridor.pro
make
./Quoridor
```

### Build with Qt Creator
1. Open `CMakeLists.txt` or `Quoridor.pro` in Qt Creator
2. Select your Qt kit (5.15+ or 6.x)
3. Press **Ctrl+R** to build and run

---

##  Controls

| Action | Control |
|--------|---------|
| Move pawn | Click **♟ Move Pawn** button (or press **W**) → click highlighted green cell |
| Place wall | Click **━ Place Wall** button (or press **W**) → hover for preview → click to place |
| Rotate wall | **Right-click** anywhere on board (while in wall mode) |
| Toggle mode | Press **W** |
| New game | Click **⟳ New Game** or press **F2** |
| Undo | Click **↩ Undo** or press **Ctrl+Z** |
| Redo | Click **↪ Redo** or press **Ctrl+Y** |
| Toggle highlights | Click **✓ Highlights** button |

---

##  AI Implementation

### Difficulty Levels

| Level | Algorithm | Behavior |
|-------|-----------|----------|
| **Easy** | Greedy BFS | Always follows shortest path to goal; **never places walls** |
| **Medium** | Greedy BFS + Selective Blocking | Moves along shortest path; places walls on opponent's path only when clearly beneficial |
| **Hard** | Minimax (depth-2) + Alpha-Beta Pruning | Strategic lookahead with evaluation function; max 12 wall candidates; 300ms time cap |

### Key AI Features
- **Immediate Win Detection**: All levels check for a winning pawn move before any computation
- **Pathfinding**: BFS-based shortest path calculation for both players
- **Evaluation Function** (Hard): `score = (opponentDistance - aiDistance) × 50` with endgame bonuses
- **Wall Candidate Generation**: Walls are selectively generated near opponent's shortest path to reduce search space

---

##  Architecture & Design

### Design Patterns Used
- **Memento Pattern** — `GameState` snapshots enable undo/redo functionality
- **Model-View Separation** — `Game` (model) is independent of Qt; `BoardWidget` (view) reads state through `Game` API
- **Strategy Pattern** — `AIPlayer` dispatches to different algorithms based on difficulty level

### Class Overview

| Class | Responsibility |
|-------|--------------|
| `Game` | Central game state, turn management, AI integration, undo/redo |
| `Board` | 9×9 grid, wall storage (2D boolean arrays), move validation, BFS pathfinding |
| `Player` | Player metadata (name, position, wall count, goal row) |
| `AIPlayer` | Computer opponent — three difficulty algorithms |
| `BoardWidget` | Qt widget: renders board, handles mouse input, emits move signals |
| `MainWindow` | Primary application window with toolbar, status panel, mode controls |
| `GameWindow` | Alternative simpler window layout |
| `GameState` | Memento struct for undo/redo snapshots |
| `Position` | Lightweight (row, col) coordinate struct |
| `Wall` | Wall piece with overlap/cross detection |

### File Structure
```
quoridor/
├── main.cpp              # Entry point with startup dialog
├── CMakeLists.txt        # CMake build config
├── Quoridor.pro          # qmake build config
├── Game.h/cpp            # Core game logic & state management
├── Board.h/cpp           # Board representation & validation
├── Player.h/cpp          # Player state
├── AIPlayer.h/cpp        # AI opponent algorithms
├── BoardWidget.h/cpp     # Qt board rendering & input
├── MainWindow.h/cpp      # Primary GUI window
├── GameWindow.h/cpp      # Alternative GUI window
├── GameState.h           # Undo/redo snapshot struct
├── Position.h            # Coordinate struct
└── Wall.h/cpp            # Wall piece logic
```

---

##  Testing & Validation

### Game Rules Validation
- Wall placement validated against overlap, crossing, and path-blocking (BFS)
- Pawn movement validated for orthogonal steps, jumps, and diagonal fallbacks
- Turn switching enforced after each valid action
- Win condition checked after every pawn move

### AI Testing
- Easy AI always finds path (no oscillation)
- Medium AI only places walls that increase opponent's path length
- Hard AI respects time cap and returns best move found

---

##  Demo Video

*[Link to demo video]*

The demo video covers:
1. Game setup and UI overview
2. Human vs Human gameplay demonstration
3. Human vs AI gameplay (all three difficulty levels)
4. Undo/redo functionality
5. Wall placement and rotation

---

##  Project Report

See `Project_Report.pdf` for detailed documentation including:
- Design decisions and architecture explanation
- Implementation challenges and solutions
- AI algorithm deep-dive
- Assumptions and limitations
- References and resources

---

##  Team Members

- *[Safie Tarek Mahmoud]* — [ID: 2300136]
- *[Moamen Ahmed Badr]* — [ID: 2300636]
- *[Ibrahim Mahrous Ahmed Mohamed]* — [ID: 2300173]
- *[Mohamed Emad Mohamed Helal]* — [ID: 2300615]


*(Update with actual team members)*

---

##  References & Resources

- [Official Quoridor Rules](https://www.gigamic.com/files/catalog/products/rules/quoridor-classic-quoridor-en.pdf)
- [Quoridor on BoardGameGeek](https://boardgamegeek.com/boardgame/624/quoridor)
- Pathfinding: BFS algorithm for shortest path validation
- AI: Minimax with Alpha-Beta Pruning for Hard difficulty
- Qt Framework Documentation (qt.io)

---

##  License

This project was developed for educational purposes as part of CSE471s: Artificial Intelligence at [University Name].

---

> **Course**: CSE472s — Artificial Intelligence  
> **Semester**: Spring 2026  
> **Instructor**: [Instructor Name]  
> **Deadline**: May 28, 2026
