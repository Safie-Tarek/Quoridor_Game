#pragma once

// ============================================================
//  Player.h
//
//  Represents one of the two players in a Quoridor game.
//
//  Responsibilities:
//    • Store the player's name and numeric index (0 or 1)
//    • Track current pawn position and starting position
//    • Track remaining wall count
//    • Know the goal row needed to win
//    • Report whether the player has won
//
//  What Player deliberately does NOT do:
//    • Validate moves     → Board is responsible
//    • Place walls        → Board is responsible
//    • Manage turns       → Game is responsible
//
//  Player is updated by Game AFTER Board confirms legality.
//  The GUI reads Player state through Game's public API.
// ============================================================

#include "Position.h"
#include <string>

class Player
{
public:

    // ---- Constants ------------------------------------------

    // Standard 2-player Quoridor gives each player 10 walls.
    static constexpr int STARTING_WALLS = 10;

    // ---- Constructors ---------------------------------------

    // Default: unnamed player at (0,0), 10 walls, goal row 0.
    Player();

    // Normal construction.
    //   name        — display name shown in the GUI ("Player 1", "CPU", …)
    //   startPos    — cell where this player's pawn begins
    //   goalRow     — row the pawn must reach to win (0 or 8)
    //   playerIndex — numeric ID: 0 or 1
    Player(const std::string& name,
           const Position&    startPos,
           int                goalRow,
           int                playerIndex);

    // ---- Getters --------------------------------------------

    const std::string& getName()        const;
    const Position&    getPosition()    const;
    int                getWallCount()   const;
    int                getGoalRow()     const;
    int                getPlayerIndex() const;

    // ---- State updates (called by Game only) ----------------

    // Updates the pawn's recorded position.
    // Game calls this after Board::applyMove() succeeds.
    void setPosition(const Position& newPos);

    // Reduces wall count by 1.
    // Game calls this after Board::applyWall() succeeds.
    // Will not go below zero.
    void useWall();

    // ---- Queries --------------------------------------------

    // True when this player still has walls left to place.
    bool hasWallsRemaining() const;

    // True when the pawn's current row equals the goal row.
    bool hasWon() const;

    // Resets pawn to starting position and restores full wall count.
    // Called by Game::resetGame().
    void reset();

    // Prints a one-line status summary to std::cout.
    // Used for console testing only.
    void printStatus() const;

private:

    std::string m_name;
    Position    m_position;     // Current pawn position
    Position    m_startPos;     // Saved for reset()
    int         m_wallCount;    // Walls remaining [0..STARTING_WALLS]
    int         m_goalRow;      // Row to reach for victory
    int         m_playerIndex;  // 0 or 1
};
