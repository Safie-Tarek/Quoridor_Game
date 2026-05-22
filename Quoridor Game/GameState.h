#pragma once

// ============================================================
//  GameState.h
//
//  A complete snapshot of the game state for undo/redo support.
//  Uses the Memento pattern: GameState is the Memento, Game is
//  the Originator, and GameHistory (in Game) is the Caretaker.
//
//  Stores everything needed to restore the game to a previous
//  point in time: board walls, pawn positions, player wall
//  counts, current turn, winner, and input mode.
// ============================================================

#include "Position.h"
#include "Wall.h"
#include <vector>

class GameState
{
public:
    // Default constructor — creates an empty state.
    GameState() = default;

    // ---- Data members (public for easy construction) --------

    // Board state
    std::vector<Wall> placedWalls;
    Position pawnPositions[2];

    // Player state
    int wallCounts[2];

    // Game flow state
    int currentPlayerIndex;
    int winnerIndex;
    int inputMode;  // stored as int to avoid Game.h dependency
    bool lastActionInvalid;

    // AI difficulty (stored so undo/redo preserves it)
    int aiDifficulty;
};
