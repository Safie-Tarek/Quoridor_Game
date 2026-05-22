#pragma once

// ============================================================
//  Game.h
//
//  The single source of truth for the entire Quoridor game.
//
//  NEW FEATURES (v2.0):
//    • AI Difficulty selection (Easy / Medium / Hard)
//    • Full Undo/Redo support via GameState snapshots (Memento pattern)
//
//  Game is the ONLY class the GUI communicates with.
//  The GUI calls Game methods to perform actions and reads
//  Game state to render the screen.
// ============================================================

#include "Board.h"
#include "Player.h"
#include "Wall.h"
#include "Position.h"
#include "GameState.h"
#include <string>
#include <vector>

// Forward declaration — AIPlayer lives in /AI but Game holds
// a pointer to it so we only need the full definition in Game.cpp.
class AIPlayer;

class Game
{
public:

    // ---- Game modes -----------------------------------------

    enum class Mode
    {
        HumanVsHuman,
        HumanVsAI
    };

    // ---- AI Difficulty levels -------------------------------
    //  NEW: selectable difficulty for the AI opponent.

    enum class AIDifficulty
    {
        Easy,    // Random legal moves
        Medium,  // Greedy shortest-path
        Hard     // Minimax with alpha-beta pruning (depth 3)
    };

    // ---- Input mode -----------------------------------------

    enum class InputMode
    {
        MovePawn,    // click selects a pawn destination
        PlaceWall    // click places a wall
    };

    // ---- Constructor / destructor ---------------------------

    // Creates a game in HumanVsHuman mode.
    // Call resetGame() or start a new Game object to change mode.
    explicit Game(Mode mode = Mode::HumanVsHuman,
                  AIDifficulty aiDiff = AIDifficulty::Medium);
    ~Game();

    // ---- Primary actions (called by GUI) --------------------

    // moveCurrentPlayer()
    //   Attempts to move the current player's pawn to 'target'.
    //   On success: pushes previous state to undo stack, clears redo stack.
    bool moveCurrentPlayer(const Position& target);

    // placeWallForCurrentPlayer()
    //   Attempts to place 'wall' for the current player.
    //   On success: pushes previous state to undo stack, clears redo stack.
    bool placeWallForCurrentPlayer(const Wall& wall);

    // resetGame()
    //   Restores the full game to its starting state.
    //   Clears both undo and redo stacks.
    void resetGame();

    // ---- Undo / Redo (NEW) ----------------------------------

    // undo()
    //   Reverts the game to the previous state.
    //   Pushes current state to redo stack, pops from undo stack.
    //   Returns true if an undo was performed.
    bool undo();

    // redo()
    //   Restores the game to the next state (after an undo).
    //   Pushes current state to undo stack, pops from redo stack.
    //   Returns true if a redo was performed.
    bool redo();

    // canUndo() / canRedo()
    //   Query whether undo/redo actions are currently available.
    bool canUndo() const;
    bool canRedo() const;

    // ---- Input mode toggle (called by GUI) ------------------

    void setInputMode(InputMode mode);
    InputMode getInputMode() const;

    // ---- AI Difficulty (NEW) --------------------------------

    void setAIDifficulty(AIDifficulty diff);
    AIDifficulty getAIDifficulty() const;
    static std::string difficultyToString(AIDifficulty diff);

    // ---- Read-only state queries (called by GUI) ------------

    const Board& getBoard() const;
    const Player& getPlayer(int index) const;
    int getCurrentPlayerIndex() const;
    const Player& getCurrentPlayer() const;
    bool isGameOver() const;
    int getWinnerIndex() const;
    Mode getMode() const;
    bool wasLastActionInvalid() const;

    std::vector<Position> getValidMovesForCurrentPlayer() const;
    std::string getStatusMessage() const;

private:

    // ---- Internal state -------------------------------------

    Board       m_board;
    Player      m_players[2];
    int         m_currentPlayerIndex;  // 0 or 1
    int         m_winnerIndex;         // -1 = no winner yet
    Mode        m_mode;
    InputMode   m_inputMode;
    bool        m_lastActionInvalid;   // true if last action was rejected
    AIDifficulty m_aiDifficulty;       // NEW: AI difficulty setting

    // AI player — nullptr in HumanVsHuman mode.
    AIPlayer*   m_aiPlayer;

    // ---- Undo/Redo history (NEW) ----------------------------
    //  Two stacks implementing the Memento pattern.
    //  undoStack: past states (most recent at back)
    //  redoStack: future states (most recent at back)
    //  Max size prevents unbounded memory growth.

    static constexpr size_t MAX_UNDO_STATES = 100;
    std::vector<GameState> m_undoStack;
    std::vector<GameState> m_redoStack;

    // ---- Private helpers ------------------------------------

    // saveState()
    //   Captures the current game state into a GameState snapshot
    //   and pushes it onto the undo stack. Clears redo stack.
    //   In HvAI mode, skips saving when it's the AI's turn.
    void saveState();

    // pushCurrentState()
    //   Captures current state and pushes to the given stack.
    void pushCurrentState(std::vector<GameState>& stack);

    // restoreFromState()
    //   Restores board, players, and game flow from a GameState.
    void restoreFromState(const GameState& state);

    // switchTurn()
    //   Moves to the next player's turn.
    //   If the next player is the AI, triggers the AI move immediately.
    void switchTurn();

    // checkForWinner()
    //   Checks whether the current player just won.
    void checkForWinner();

    // triggerAIMove()
    //   If it is the AI's turn, asks AIPlayer for a move and applies it.
    void triggerAIMove();

    // initPlayers()
    //   Sets up the two Player objects with correct names, positions,
    //   goal rows, and indices.
    void initPlayers();

    // createAIPlayer()
    //   Creates/destroys the AI player based on mode and difficulty.
    void createAIPlayer();
};
