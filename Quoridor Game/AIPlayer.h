#pragma once

// ============================================================
//  AIPlayer.h
//
//  Computes moves for the computer opponent.
//
//  Three difficulty levels:
//
//  Easy:
//    Picks a random legal move from getValidMoves().
//    Occasionally places a wall at random just to mix things up.
//
//  Medium:
//    Always advances the pawn along the BFS shortest path to
//    its goal row.  Only places a wall if it significantly
//    widens the gap between the two players' distances.
//
//  Hard:
//    Minimax search with alpha-beta pruning to depth 2.
//    Evaluates positions using BFS distances for both players.
//    Evaluation: score = opponentBfsDistance - aiBfsDistance
//    Higher score = AI is relatively closer to winning.
//
//  AIPlayer is a pure logic class with no Qt dependencies.
//  Game calls getBestMove() and feeds the result back to Board.
// ============================================================

#include "Board.h"
#include "Position.h"
#include "Wall.h"
#include <vector>
#include <chrono>

// A Move is either a pawn move or a wall placement.
// We use a tagged union-style struct for clarity.
struct AIMove
{
    enum class Type { Pawn, Wall } type;
    Position    pawnTarget;   // valid when type == Pawn
    Wall        wall;         // valid when type == Wall

    // Convenience factories.
    static AIMove pawnMove(const Position& target)
    {
        AIMove m;
        m.type       = Type::Pawn;
        m.pawnTarget = target;
        return m;
    }
    static AIMove wallMove(const Wall& w)
    {
        AIMove m;
        m.type = Type::Wall;
        m.wall = w;
        return m;
    }
};

class AIPlayer
{
public:

    enum class Difficulty { Easy, Medium, Hard };

    // aiIndex    — which player the AI controls (0 or 1)
    // difficulty — Easy / Medium / Hard
    explicit AIPlayer(int aiIndex, Difficulty difficulty = Difficulty::Medium);

    // getBestMove()
    //   Given the current board and wall counts, returns the AI's
    //   chosen move.  This is the only method Game needs to call.
    //
    //   Parameters:
    //     board       — current board state (read-only)
    //     aiWalls     — walls remaining for the AI player
    //     humanWalls  — walls remaining for the human player
    AIMove getBestMove(const Board& board, int aiWalls, int humanWalls) const;

private:

    int        m_aiIndex;      // 0 or 1 — which player we are
    int        m_humanIndex;   // the other player
    Difficulty m_difficulty;

    // ---- Timing (mutable because getBestMove is const) ----
    mutable std::chrono::steady_clock::time_point m_startTime;
    int                                           m_timeLimitMs;

    bool timeExceeded() const;

    // If a pawn move reaches the goal row immediately, return it.
    Position findWinningPawnMove(const Board& board, int playerIndex) const;

    // ---- Easy -----------------------------------------------
    AIMove easyMove(const Board& board) const;

    // ---- Medium ---------------------------------------------
    AIMove mediumMove(const Board& board, int aiWalls) const;

    // Helper: best pawn move using greedy BFS (used by Medium & Hard)
    AIMove getBestPawnMove(const Board& board) const;

    // ---- Hard (Minimax + alpha-beta) ------------------------

    // Entry point: tries all moves, returns the one with highest score.
    AIMove hardMove(const Board& board, int aiWalls, int humanWalls) const;

    // minimax()
    //   Recursive Minimax with alpha-beta pruning.
    //
    //   board        — board state at this node (copied per call)
    //   depth        — remaining search depth
    //   alpha        — best score the maximiser is guaranteed so far
    //   beta         — best score the minimiser is guaranteed so far
    //   isMaximising — true when it is the AI's turn at this node
    //   aiWalls      — AI's wall count at this node
    //   humanWalls   — human's wall count at this node
    //
    //   Returns the heuristic score of the position.
    int minimax(Board board,
                int   depth,
                int   alpha,
                int   beta,
                bool  isMaximising,
                int   aiWalls,
                int   humanWalls) const;

    // evaluate()
    //   Lightweight heuristic score for a board position.
    //   Score = humanBfsDistance - aiBfsDistance.
    int evaluate(const Board& board) const;

    // generateHardMoves()
    //   Produces all legal moves for the given player:
    //     • all valid pawn moves
    //     • up to 12 selective wall placements (if wallCount > 0)
    //   Used inside minimax to expand nodes.
    std::vector<AIMove> generateHardMoves(const Board& board,
                                            int           playerIndex,
                                            int           wallCount) const;

    // applyAIMove()
    //   Applies an AIMove to a board copy for minimax simulation.
    //   Returns false if the move was invalid (safety guard).
    bool applyAIMove(Board& board, const AIMove& move,
                     int playerIndex, int& wallCount) const;
};
