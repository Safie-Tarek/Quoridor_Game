// ============================================================
//  Game.cpp
//
//  Implementation of the Game class — the single source of
//  truth for the entire Quoridor game state.
//
//  NEW (v2.1):
//    • In HvAI mode, undo/redo only controls PLAYER moves.
//      AI moves are "atomic" with the preceding player move.
//    • saveState() skips saving when it's the AI's turn.
// ============================================================

#include "Game.h"
#include "AIPlayer.h"

// ============================================================
//  Constructor
// ============================================================
Game::Game(Mode mode, AIDifficulty aiDiff)
    : m_currentPlayerIndex(0)
    , m_winnerIndex(-1)
    , m_mode(mode)
    , m_inputMode(InputMode::MovePawn)
    , m_lastActionInvalid(false)
    , m_aiDifficulty(aiDiff)
    , m_aiPlayer(nullptr)
{
    initPlayers();
    createAIPlayer();

    // Push initial state onto undo stack so we can undo to start.
    saveState();
}

// ============================================================
//  Destructor
// ============================================================
Game::~Game()
{
    delete m_aiPlayer;
}

// ============================================================
//  saveState()  [private]
//
//  Captures current game state and pushes to undo stack.
//  Clears redo stack (new branch in history).
//
//  CRITICAL: In HvAI mode, we do NOT save states where it is
//  the AI's turn.  This ensures undo/redo only controls the
//  human player's moves — the AI response is treated as an
//  atomic consequence of the human move.
// ============================================================
void Game::saveState()
{
    // Skip saving AI-turn states in HumanVsAI mode.
    // Player 1 is always the AI in HvAI mode.
    if (m_mode == Mode::HumanVsAI && m_currentPlayerIndex == 1)
        return;

    GameState state;

    // Board state
    state.placedWalls = m_board.getPlacedWalls();
    state.pawnPositions[0] = m_board.getPawnPosition(0);
    state.pawnPositions[1] = m_board.getPawnPosition(1);

    // Player state
    state.wallCounts[0] = m_players[0].getWallCount();
    state.wallCounts[1] = m_players[1].getWallCount();

    // Game flow
    state.currentPlayerIndex = m_currentPlayerIndex;
    state.winnerIndex = m_winnerIndex;
    state.inputMode = static_cast<int>(m_inputMode);
    state.lastActionInvalid = m_lastActionInvalid;
    state.aiDifficulty = static_cast<int>(m_aiDifficulty);

    m_undoStack.push_back(state);

    // Enforce max undo history.
    if (m_undoStack.size() > MAX_UNDO_STATES)
        m_undoStack.erase(m_undoStack.begin());

    // New action invalidates any redo history.
    m_redoStack.clear();
}

// ============================================================
//  moveCurrentPlayer()
// ============================================================
bool Game::moveCurrentPlayer(const Position& target)
{
    if (isGameOver())
    {
        m_lastActionInvalid = true;
        return false;
    }

    Player& current = m_players[m_currentPlayerIndex];

    if (!m_board.isMoveValid(m_currentPlayerIndex, target))
    {
        m_lastActionInvalid = true;
        return false;
    }

    // Save state BEFORE applying the move (for undo).
    saveState();

    m_board.applyMove(m_currentPlayerIndex, target);
    current.setPosition(target);
    m_lastActionInvalid = false;

    checkForWinner();
    if (!isGameOver())
        switchTurn();

    return true;
}

// ============================================================
//  placeWallForCurrentPlayer()
// ============================================================
bool Game::placeWallForCurrentPlayer(const Wall& wall)
{
    if (isGameOver())
    {
        m_lastActionInvalid = true;
        return false;
    }

    Player& current = m_players[m_currentPlayerIndex];

    if (!current.hasWallsRemaining())
    {
        m_lastActionInvalid = true;
        return false;
    }

    if (!m_board.isWallValid(wall))
    {
        m_lastActionInvalid = true;
        return false;
    }

    // Save state BEFORE applying the wall (for undo).
    saveState();

    m_board.applyWall(wall);
    current.useWall();
    m_lastActionInvalid = false;
    switchTurn();

    return true;
}

// ============================================================
//  resetGame()
// ============================================================
void Game::resetGame()
{
    m_board.reset();
    m_currentPlayerIndex = 0;
    m_winnerIndex = -1;
    m_lastActionInvalid = false;
    m_inputMode = InputMode::MovePawn;

    m_players[0].reset();
    m_players[1].reset();

    // Clear history stacks.
    m_undoStack.clear();
    m_redoStack.clear();

    // Save initial state.
    saveState();
}

// ============================================================
//  Undo / Redo
//
//  In HvH mode: each undo/redo steps back/forward one turn.
//  In HvAI mode: each undo/redo steps back/forward one HUMAN
//  turn (the AI response is undone/redone atomically).
// ============================================================
bool Game::undo()
{
    if (!canUndo()) return false;

    // Save current state to redo stack.
    pushCurrentState(m_redoStack);

    // Pop previous state from undo stack and restore it.
    GameState previous = m_undoStack.back();
    m_undoStack.pop_back();

    restoreFromState(previous);
    return true;
}

bool Game::redo()
{
    if (!canRedo()) return false;

    // Save current state to undo stack.
    pushCurrentState(m_undoStack);

    // Pop next state from redo stack and restore it.
    GameState next = m_redoStack.back();
    m_redoStack.pop_back();

    restoreFromState(next);
    return true;
}

// ============================================================
//  pushCurrentState()  [private helper]
//  Captures current state and pushes to the given stack.
// ============================================================
void Game::pushCurrentState(std::vector<GameState>& stack)
{
    GameState current;
    current.placedWalls = m_board.getPlacedWalls();
    current.pawnPositions[0] = m_board.getPawnPosition(0);
    current.pawnPositions[1] = m_board.getPawnPosition(1);
    current.wallCounts[0] = m_players[0].getWallCount();
    current.wallCounts[1] = m_players[1].getWallCount();
    current.currentPlayerIndex = m_currentPlayerIndex;
    current.winnerIndex = m_winnerIndex;
    current.inputMode = static_cast<int>(m_inputMode);
    current.lastActionInvalid = m_lastActionInvalid;
    current.aiDifficulty = static_cast<int>(m_aiDifficulty);

    stack.push_back(current);

    if (stack.size() > MAX_UNDO_STATES)
        stack.erase(stack.begin());
}

// ============================================================
//  restoreFromState()  [private helper]
//  Restores all game data from a GameState snapshot.
// ============================================================
void Game::restoreFromState(const GameState& state)
{
    // Restore board walls
    m_board.reset();
    for (const Wall& w : state.placedWalls)
        m_board.applyWall(w);

    // Restore pawn positions
    m_board.applyMove(0, state.pawnPositions[0]);
    m_board.applyMove(1, state.pawnPositions[1]);

    // Restore players
    m_players[0].reset();
    m_players[1].reset();
    m_players[0].setPosition(state.pawnPositions[0]);
    m_players[1].setPosition(state.pawnPositions[1]);

    int wallsUsed0 = Player::STARTING_WALLS - state.wallCounts[0];
    int wallsUsed1 = Player::STARTING_WALLS - state.wallCounts[1];
    for (int i = 0; i < wallsUsed0; ++i) m_players[0].useWall();
    for (int i = 0; i < wallsUsed1; ++i) m_players[1].useWall();

    // Restore game flow
    m_currentPlayerIndex = state.currentPlayerIndex;
    m_winnerIndex = state.winnerIndex;
    m_inputMode = static_cast<InputMode>(state.inputMode);
    m_lastActionInvalid = state.lastActionInvalid;
}

bool Game::canUndo() const
{
    // undoStack always contains the initial state at index 0.
    // We can undo when there's at least one state before current.
    return m_undoStack.size() > 1;
}

bool Game::canRedo() const
{
    return !m_redoStack.empty();
}

// ============================================================
//  Input mode
// ============================================================
void Game::setInputMode(InputMode mode) { m_inputMode = mode; }
Game::InputMode Game::getInputMode() const { return m_inputMode; }

// ============================================================
//  AI Difficulty
// ============================================================
void Game::setAIDifficulty(AIDifficulty diff)
{
    m_aiDifficulty = diff;
    if (m_mode == Mode::HumanVsAI)
        createAIPlayer();
}

Game::AIDifficulty Game::getAIDifficulty() const
{
    return m_aiDifficulty;
}

std::string Game::difficultyToString(AIDifficulty diff)
{
    switch (diff)
    {
        case AIDifficulty::Easy:   return "Easy";
        case AIDifficulty::Medium: return "Medium";
        case AIDifficulty::Hard:   return "Hard";
    }
    return "Medium";
}

// ============================================================
//  Read-only state queries
// ============================================================
const Board&  Game::getBoard()               const { return m_board; }
const Player& Game::getPlayer(int index)     const { return m_players[index]; }
int           Game::getCurrentPlayerIndex()  const { return m_currentPlayerIndex; }
const Player& Game::getCurrentPlayer()       const { return m_players[m_currentPlayerIndex]; }
bool          Game::isGameOver()             const { return m_winnerIndex != -1; }
int           Game::getWinnerIndex()         const { return m_winnerIndex; }
Game::Mode    Game::getMode()                const { return m_mode; }
bool          Game::wasLastActionInvalid()   const { return m_lastActionInvalid; }

std::vector<Position> Game::getValidMovesForCurrentPlayer() const
{
    return m_board.getValidMoves(m_currentPlayerIndex);
}

std::string Game::getStatusMessage() const
{
    if (isGameOver())
    {
        return m_players[m_winnerIndex].getName() + " wins!";
    }

    std::string msg = m_players[m_currentPlayerIndex].getName() + "'s turn";

    if (m_inputMode == InputMode::PlaceWall)
        msg += "  [Wall mode — press W to switch]";
    else
        msg += "  [Move mode — press W to switch]";

    if (m_lastActionInvalid)
        msg += "  ⚠ Invalid move";

    return msg;
}

// ============================================================
//  switchTurn()  [private]
// ============================================================
void Game::switchTurn()
{
    m_currentPlayerIndex = 1 - m_currentPlayerIndex;

    if (m_mode == Mode::HumanVsAI && m_currentPlayerIndex == 1)
        triggerAIMove();
}

// ============================================================
//  checkForWinner()  [private]
// ============================================================
void Game::checkForWinner()
{
    for (int i = 0; i < 2; ++i)
    {
        if (m_board.hasPlayerWon(i))
        {
            m_winnerIndex = i;
            return;
        }
    }
}

// ============================================================
//  triggerAIMove()  [private]
//
//  Asks the AIPlayer for its best move and applies it.
//  NO saveState() call here — AI moves are NOT independently
//  undoable.  They are part of the preceding human move.
// ============================================================
void Game::triggerAIMove()
{
    if (!m_aiPlayer || isGameOver()) return;

    int aiWalls    = m_players[1].getWallCount();
    int humanWalls = m_players[0].getWallCount();

    AIMove move = m_aiPlayer->getBestMove(m_board, aiWalls, humanWalls);

    if (move.type == AIMove::Type::Pawn)
    {
        if (m_board.isMoveValid(1, move.pawnTarget))
        {
            m_board.applyMove(1, move.pawnTarget);
            m_players[1].setPosition(move.pawnTarget);
            checkForWinner();
            if (!isGameOver())
                m_currentPlayerIndex = 0;   // back to human
        }
    }
    else // Wall
    {
        if (m_players[1].hasWallsRemaining() &&
            m_board.isWallValid(move.wall))
        {
            m_board.applyWall(move.wall);
            m_players[1].useWall();
            m_currentPlayerIndex = 0;
        }
    }
}

// ============================================================
//  initPlayers()  [private]
// ============================================================
void Game::initPlayers()
{
    m_players[0] = Player("Player 1",
                          Position(Board::P0_START_ROW, Board::P0_START_COL),
                          Board::P0_GOAL_ROW,
                          0);

    std::string p1Name = (m_mode == Mode::HumanVsAI) ? "CPU" : "Player 2";
    m_players[1] = Player(p1Name,
                          Position(Board::P1_START_ROW, Board::P1_START_COL),
                          Board::P1_GOAL_ROW,
                          1);
}

// ============================================================
//  createAIPlayer()  [private]
// ============================================================
void Game::createAIPlayer()
{
    delete m_aiPlayer;
    m_aiPlayer = nullptr;

    if (m_mode == Mode::HumanVsAI)
    {
        AIPlayer::Difficulty diff;
        switch (m_aiDifficulty)
        {
            case AIDifficulty::Easy:   diff = AIPlayer::Difficulty::Easy;   break;
            case AIDifficulty::Medium: diff = AIPlayer::Difficulty::Medium; break;
            case AIDifficulty::Hard:   diff = AIPlayer::Difficulty::Hard;   break;
        }
        m_aiPlayer = new AIPlayer(1, diff);
    }
}
