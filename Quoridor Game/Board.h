#pragma once

// ============================================================
//  Board.h
//
//  Represents the complete physical state of the 9x9 Quoridor
//  board — walls, pawn positions, and all related validation.
//
//  Responsibilities:
//    • Store and update pawn positions
//    • Store placed walls via two 2-D boolean arrays
//    • Validate pawn movement (orthogonal + jump rules)
//    • Validate wall placement (bounds, overlap, cross, BFS)
//    • Expose a clean read-only API for the GUI
//
//  What Board deliberately does NOT do:
//    • Track whose turn it is       → Game's responsibility
//    • Track wall counts per player → Player's responsibility
//    • Make AI decisions            → AIPlayer's responsibility
//
//  Internal wall storage
//  ---------------------
//  Instead of a Cell class we use two flat arrays:
//
//    bool m_wallSouth[9][9]
//      true → wall on SOUTH edge of cell (r,c)
//             pawn cannot cross row r → row r+1 at column c
//
//    bool m_wallEast[9][9]
//      true → wall on EAST edge of cell (r,c)
//             pawn cannot cross col c → col c+1 at row r
//
//  Player convention:
//    Index 0 → starts (8,4), goal row 0  (moves upward)
//    Index 1 → starts (0,4), goal row 8  (moves downward)
//
//  Refactor notes (vs original):
//    • isEdgeBlocked() replaces isWallBlockingEdge() — shorter, clearer
//    • getNeighbors() → getPassableNeighbors() — name explains what it does
//    • Added full jump support: straight jump + diagonal fallback
//    • bfsDistance() added alongside hasPathToGoal() for AI use
//    • All directions expressed as a static constexpr delta table
// ============================================================

#include "Position.h"
#include "Wall.h"
#include <vector>

class Board
{
public:

    // ---- Constants ------------------------------------------

    static constexpr int SIZE        = 9;    // board is SIZE x SIZE
    static constexpr int NUM_PLAYERS = 2;

    // Starting positions (centre of each baseline)
    static constexpr int P0_START_ROW = 8;
    static constexpr int P0_START_COL = 4;
    static constexpr int P1_START_ROW = 0;
    static constexpr int P1_START_COL = 4;

    // Goal rows
    static constexpr int P0_GOAL_ROW = 0;   // Player 0 must reach top
    static constexpr int P1_GOAL_ROW = 8;   // Player 1 must reach bottom

    // ---- Four orthogonal direction deltas -------------------
    //   Used by movement validation and BFS.
    //   Stored as a fixed array so loops over directions are uniform.
    static constexpr int DIRECTIONS[4][2] = {
        {-1,  0},   // North (up)
        {+1,  0},   // South (down)
        { 0, -1},   // West  (left)
        { 0, +1}    // East  (right)
    };

    // ---- Constructor / reset --------------------------------

    // Sets up a fresh board: no walls, pawns at starting positions.
    Board();

    // Restores the board to its initial state.
    // Called by Game::resetGame().
    void reset();

    // ---- Pawn movement API ----------------------------------

    // isMoveValid()
    //   Returns true if moving playerIndex's pawn to 'target'
    //   is legal under the current board state.
    //
    //   Rules checked:
    //     1. playerIndex is 0 or 1
    //     2. 'target' is inside the 9x9 grid
    //     3. 'target' is one orthogonal step away  OR
    //        a legal jump (straight or diagonal)
    //     4. No wall blocks the path
    //
    //   Marked const — never modifies board state.
    bool isMoveValid(int playerIndex, const Position& target) const;

    // applyMove()
    //   Moves playerIndex's pawn to 'target' without re-validating.
    //   ONLY call this after isMoveValid() returns true.
    //   Game::moveCurrentPlayer() is responsible for the validation step.
    void applyMove(int playerIndex, const Position& target);

    // getValidMoves()
    //   Returns all legal destinations for playerIndex's pawn.
    //   Used by the GUI for move highlighting and by the AI for
    //   move generation.
    std::vector<Position> getValidMoves(int playerIndex) const;

    // ---- Wall placement API ---------------------------------

    // isWallValid()
    //   Returns true if placing 'wall' is legal:
    //     1. Anchor within [0..7] x [0..7]
    //     2. No overlap with existing walls
    //     3. No crossing with existing walls
    //     4. Both players still have a path to their goal (BFS)
    //
    //   Uses a temporary board copy for the BFS step so the
    //   live board is never modified.
    bool isWallValid(const Wall& wall) const;

    // applyWall()
    //   Places 'wall' on the board without re-validating.
    //   ONLY call after isWallValid() returns true.
    void applyWall(const Wall& wall);

    // ---- State queries (read-only) --------------------------

    Position               getPawnPosition(int playerIndex) const;
    const std::vector<Wall>& getPlacedWalls()               const;

    // Returns true if playerIndex's pawn has reached their goal row.
    bool hasPlayerWon(int playerIndex) const;

    // isEdgeBlocked()
    //   Returns true if the edge between 'from' and the cell in
    //   direction (dr, dc) is blocked by a wall.
    //   Exposed publicly so the GUI can draw wall indicators.
    //
    //   (dr, dc) must be one of: (-1,0) (1,0) (0,-1) (0,1)
    bool isEdgeBlocked(const Position& from, int dr, int dc) const;

    // bfsDistance()
    //   Returns the shortest number of pawn moves from 'start'
    //   to 'goalRow', ignoring the opponent's pawn.
    //   Returns -1 if no path exists (used by isWallValid).
    //   Used by the AI evaluation function.
    int bfsDistance(const Position& start, int goalRow) const;

    // getShortestPath()
    //   Returns the sequence of positions forming the shortest path
    //   from 'start' to 'goalRow'.  Empty if no path exists.
    //   Used by the AI to find critical wall placement spots.
    std::vector<Position> getShortestPath(const Position& start, int goalRow) const;

    // printToConsole()
    //   ASCII snapshot for console testing only.
    //   Not called by the Qt GUI.
    void printToConsole() const;

private:

    // ---- Internal state -------------------------------------

    // Wall presence arrays — see class header for encoding.
    bool m_wallSouth[SIZE][SIZE];
    bool m_wallEast [SIZE][SIZE];

    // Current pawn positions: index 0 = Player 0, index 1 = Player 1.
    Position m_pawnPos[NUM_PLAYERS];

    // All placed walls — used for overlap/crossing checks and GUI rendering.
    std::vector<Wall> m_placedWalls;

    // ---- Private helpers ------------------------------------

    // writeWallToArrays()
    //   Marks the two cell-edges that 'wall' covers in m_wallSouth /
    //   m_wallEast.  Called by applyWall() and by isWallValid() on
    //   a temporary copy.
    void writeWallToArrays(const Wall& wall);

    // getPassableNeighbors()
    //   Returns all cells reachable in one orthogonal step from 'pos',
    //   considering only walls (not the other pawn).
    //   The core adjacency function used by BFS.
    std::vector<Position> getPassableNeighbors(const Position& pos) const;

    // isSimpleMoveValid()
    //   Checks a plain one-step orthogonal move (no jumping).
    //   Used internally by isMoveValid().
    bool isSimpleMoveValid(const Position& from, const Position& to) const;

    // isJumpMoveValid()
    //   Checks the jump rules:
    //     • If the adjacent cell contains the opponent's pawn,
    //       a straight jump is allowed if not wall-blocked behind them.
    //     • If the straight jump is blocked by a wall, diagonal
    //       jumps to the sides of the opponent are allowed.
    //   Used internally by isMoveValid().
    bool isJumpMoveValid(int playerIndex,
                         const Position& from,
                         const Position& to) const;
};
