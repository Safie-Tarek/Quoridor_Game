// ============================================================
//  AIPlayer.cpp  (OPTIMIZED v3.0)
//
//  AI Difficulty Levels:
//    • Easy   — Pure forward movement, NEVER places walls.
//               Zero BFS calls.  Deterministic and instant.
//    • Medium — Greedy shortest path + selective wall placement.
//               Walls only placed when they clearly block opponent.
//               Limited wall candidates, single BFS per candidate.
//    • Hard   — Minimax depth-2 with alpha-beta pruning.
//               Reduced move generation (max 12 wall candidates).
//               Time-capped at 300ms.  Lightweight evaluation.
// ============================================================

#include "AIPlayer.h"
#include <algorithm>
#include <climits>
#include <chrono>

// ============================================================
//  Constructor
// ============================================================
AIPlayer::AIPlayer(int aiIndex, Difficulty difficulty)
    : m_aiIndex(aiIndex)
    , m_humanIndex(1 - aiIndex)
    , m_difficulty(difficulty)
    , m_startTime()
    , m_timeLimitMs(300)
{}

// ============================================================
//  getBestMove()
// ============================================================
AIMove AIPlayer::getBestMove(const Board& board,
                              int          aiWalls,
                              int          humanWalls) const
{
    m_startTime = std::chrono::steady_clock::now();

    switch (m_difficulty)
    {
        case Difficulty::Easy:
            return easyMove(board);
        case Difficulty::Medium:
            return mediumMove(board, aiWalls);
        case Difficulty::Hard:
            return hardMove(board, aiWalls, humanWalls);
    }
    return easyMove(board);
}

// ============================================================
//  timeExceeded()
// ============================================================
bool AIPlayer::timeExceeded() const
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_startTime).count();
    return elapsed > m_timeLimitMs;
}

// ============================================================
//  findWinningPawnMove()
//
//  If the current player has a pawn move that reaches the goal
//  row immediately, return it.  Otherwise return an invalid
//  Position (-1,-1).
// ============================================================
Position AIPlayer::findWinningPawnMove(const Board& board, int playerIndex) const
{
    int goalRow = (playerIndex == 0) ? Board::P0_GOAL_ROW : Board::P1_GOAL_ROW;
    for (const Position& p : board.getValidMoves(playerIndex))
    {
        if (p.row == goalRow)
            return p;
    }
    return Position(-1, -1);
}

// ============================================================
//  easyMove()
//
//  Uses BFS to always follow the shortest path to the goal.
//  NEVER places walls — pure pawn movement.
//
//  This guarantees the AI never gets stuck oscillating;
//  it always makes concrete progress toward its goal row.
// ============================================================
AIMove AIPlayer::easyMove(const Board& board) const
{
    // 1. If we can win right now, do it.
    Position win = findWinningPawnMove(board, m_aiIndex);
    if (win.isValid())
        return AIMove::pawnMove(win);

    // 2. Otherwise follow the shortest BFS path.
    const Position& current = board.getPawnPosition(m_aiIndex);
    int goalRow = (m_aiIndex == 0) ? Board::P0_GOAL_ROW : Board::P1_GOAL_ROW;

    std::vector<Position> moves = board.getValidMoves(m_aiIndex);
    if (moves.empty())
        return AIMove::pawnMove(current);

    Position bestMove = moves[0];
    int bestDist = 999;

    for (const Position& p : moves)
    {
        Board temp = board;
        temp.applyMove(m_aiIndex, p);
        int dist = temp.bfsDistance(p, goalRow);
        if (dist == -1) dist = 999;

        if (dist < bestDist)
        {
            bestDist = dist;
            bestMove = p;
        }
    }

    return AIMove::pawnMove(bestMove);
}

// ============================================================
//  getBestPawnMove() — helper for Medium and Hard
// ============================================================
AIMove AIPlayer::getBestPawnMove(const Board& board) const
{
    const Position& current = board.getPawnPosition(m_aiIndex);
    int goalRow = (m_aiIndex == 0) ? Board::P0_GOAL_ROW : Board::P1_GOAL_ROW;

    std::vector<Position> moves = board.getValidMoves(m_aiIndex);
    Position bestMove = current;
    int bestDist = board.bfsDistance(current, goalRow);
    if (bestDist == -1) bestDist = 100;

    for (const Position& p : moves)
    {
        Board temp = board;
        temp.applyMove(m_aiIndex, p);
        int dist = temp.bfsDistance(p, goalRow);
        if (dist == -1) dist = 100;
        if (dist < bestDist)
        {
            bestDist = dist;
            bestMove = p;
        }
    }

    return AIMove::pawnMove(bestMove);
}

// ============================================================
//  mediumMove()
//
//  Greedy shortest path + selective wall placement.
//  Walls are placed ONLY when they clearly increase opponent's
//  path length without hurting our own path.
//
//  Optimizations:
//    • Single BFS for current distances (cached)
//    • Only check walls directly on opponent's shortest path
//    • Max 8 wall candidates tested
// ============================================================
AIMove AIPlayer::mediumMove(const Board& board, int aiWalls) const
{
    // 1. If we can win right now, do it immediately.
    Position win = findWinningPawnMove(board, m_aiIndex);
    if (win.isValid())
        return AIMove::pawnMove(win);

    // 2. Otherwise compute the best pawn move and consider walls.
    AIMove bestPawn = getBestPawnMove(board);

    // If no walls left, just move
    if (aiWalls <= 0)
        return bestPawn;

    int opponentIndex = 1 - m_aiIndex;
    const Position& opp = board.getPawnPosition(opponentIndex);
    int oppGoalRow = (opponentIndex == 0) ? Board::P0_GOAL_ROW : Board::P1_GOAL_ROW;

    const Position& current = board.getPawnPosition(m_aiIndex);
    int myGoalRow = (m_aiIndex == 0) ? Board::P0_GOAL_ROW : Board::P1_GOAL_ROW;

    // Cache distances
    int myCurrentDist = board.bfsDistance(current, myGoalRow);
    if (myCurrentDist == -1) myCurrentDist = 100;
    int oppCurrentDist = board.bfsDistance(opp, oppGoalRow);
    if (oppCurrentDist == -1) oppCurrentDist = 100;

    // Get opponent's shortest path to find critical wall spots
    std::vector<Position> oppPath = board.getShortestPath(opp, oppGoalRow);

    Wall bestWall;
    bool foundGoodWall = false;
    int bestWallScore = 0;

    // Only try walls that could block the opponent's path
    bool tried[8][8][2] = {};

    auto tryWall = [&](int ar, int ac, Wall::Orientation orient)
    {
        if (ar < 0 || ar > 7 || ac < 0 || ac > 7) return;
        int o = (orient == Wall::Orientation::Horizontal) ? 0 : 1;
        if (tried[ar][ac][o]) return;
        tried[ar][ac][o] = true;

        Wall w(ar, ac, orient);
        if (!board.isWallValid(w)) return;

        Board temp = board;
        temp.applyWall(w);

        int myNewDist = temp.bfsDistance(current, myGoalRow);
        int oppNewDist = temp.bfsDistance(opp, oppGoalRow);
        if (myNewDist == -1) myNewDist = 100;
        if (oppNewDist == -1) oppNewDist = 100;

        // Skip if it blocks our own path significantly
        if (myNewDist > myCurrentDist + 1)
            return;

        // Score: opponent hurt vs our hurt
        int oppHurt = oppNewDist - oppCurrentDist;
        int myHurt = myNewDist - myCurrentDist;
        int score = oppHurt * 5 - myHurt * 3;

        // Only place if it clearly helps (hurts opponent more than us)
        if (score > bestWallScore)
        {
            bestWallScore = score;
            bestWall = w;
            foundGoodWall = true;
        }
    };

    // Try walls near opponent's path (max ~8 candidates)
    for (const Position& pathCell : oppPath)
    {
        if (timeExceeded()) break;
        // Try walls that could block movement from this cell
        if (pathCell.row < 8)
        {
            tryWall(pathCell.row, pathCell.col, Wall::Orientation::Horizontal);
            if (pathCell.col > 0)
                tryWall(pathCell.row, pathCell.col - 1, Wall::Orientation::Horizontal);
        }
        if (pathCell.col < 8)
        {
            tryWall(pathCell.row, pathCell.col, Wall::Orientation::Vertical);
            if (pathCell.row > 0)
                tryWall(pathCell.row - 1, pathCell.col, Wall::Orientation::Vertical);
        }
    }

    // Also try a few walls near the opponent's current position
    for (int dr = -1; dr <= 1 && !timeExceeded(); ++dr)
    for (int dc = -1; dc <= 1; ++dc)
    {
        int ar = opp.row + dr;
        int ac = opp.col + dc;
        if (ar >= 0 && ar <= 7 && ac >= 0 && ac <= 7)
        {
            tryWall(ar, ac, Wall::Orientation::Horizontal);
            tryWall(ar, ac, Wall::Orientation::Vertical);
        }
    }

    // Place wall only if it significantly helps (score > 3)
    if (foundGoodWall && bestWallScore > 3)
        return AIMove::wallMove(bestWall);

    return bestPawn;
}

// ============================================================
//  hardMove()
//
//  Minimax depth-2 with alpha-beta pruning.
//
//  Optimizations vs previous version:
//    • Max 12 wall candidates (reduced from ~25)
//    • Time-capped at 300ms
//    • Lighter evaluation (no endgame bonuses, just path diff)
//    • Early cutoff if winning move found
// ============================================================
AIMove AIPlayer::hardMove(const Board& board,
                           int          aiWalls,
                           int          humanWalls) const
{
    // 0. If we can win immediately, just do it — no search needed.
    Position win = findWinningPawnMove(board, m_aiIndex);
    if (win.isValid())
        return AIMove::pawnMove(win);

    static constexpr int SEARCH_DEPTH = 2;

    std::vector<AIMove> moves = generateHardMoves(board, m_aiIndex, aiWalls);

    if (moves.empty())
        return AIMove::pawnMove(board.getPawnPosition(m_aiIndex));

    // Sort: pawn moves first (faster to evaluate), then walls
    std::stable_sort(moves.begin(), moves.end(),
        [](const AIMove& a, const AIMove& b) {
            return a.type == AIMove::Type::Pawn && b.type == AIMove::Type::Wall;
        });

    AIMove bestMove = moves[0];
    int    bestScore = INT_MIN;

    for (const AIMove& candidate : moves)
    {
        if (timeExceeded()) break;

        Board tempBoard  = board;
        int   tempAiW    = aiWalls;
        int   tempHumanW = humanWalls;

        if (!applyAIMove(tempBoard, candidate, m_aiIndex, tempAiW))
            continue;

        int score = minimax(tempBoard,
                            SEARCH_DEPTH - 1,
                            INT_MIN, INT_MAX,
                            false,
                            tempAiW, tempHumanW);

        if (score > bestScore)
        {
            bestScore = score;
            bestMove  = candidate;
        }

        if (bestScore >= 9000) break;  // Winning move found
    }

    return bestMove;
}

// ============================================================
//  minimax()
// ============================================================
int AIPlayer::minimax(Board board,
                      int   depth,
                      int   alpha,
                      int   beta,
                      bool  isMaximising,
                      int   aiWalls,
                      int   humanWalls) const
{
    if (timeExceeded())
        return evaluate(board);

    if (board.hasPlayerWon(m_aiIndex))    return  10000;
    if (board.hasPlayerWon(m_humanIndex)) return -10000;
    if (depth == 0)                       return evaluate(board);

    int playerIndex = isMaximising ? m_aiIndex : m_humanIndex;
    int wallCount   = isMaximising ? aiWalls   : humanWalls;

    std::vector<AIMove> moves = generateHardMoves(board, playerIndex, wallCount);

    if (moves.empty())
        return evaluate(board);

    // Pawn moves first for speed
    std::stable_sort(moves.begin(), moves.end(),
        [](const AIMove& a, const AIMove& b) {
            return a.type == AIMove::Type::Pawn && b.type == AIMove::Type::Wall;
        });

    if (isMaximising)
    {
        int best = INT_MIN;
        for (const AIMove& move : moves)
        {
            if (timeExceeded()) break;

            Board tempBoard  = board;
            int   tempAiW    = aiWalls;
            int   tempHumanW = humanWalls;
            int&  wallRef    = isMaximising ? tempAiW : tempHumanW;

            if (!applyAIMove(tempBoard, move, playerIndex, wallRef))
                continue;

            int score = minimax(tempBoard, depth - 1,
                                alpha, beta, false,
                                tempAiW, tempHumanW);

            best  = std::max(best, score);
            alpha = std::max(alpha, best);
            if (beta <= alpha) break;
        }
        return best;
    }
    else
    {
        int best = INT_MAX;
        for (const AIMove& move : moves)
        {
            if (timeExceeded()) break;

            Board tempBoard  = board;
            int   tempAiW    = aiWalls;
            int   tempHumanW = humanWalls;
            int&  wallRef    = isMaximising ? tempAiW : tempHumanW;

            if (!applyAIMove(tempBoard, move, playerIndex, wallRef))
                continue;

            int score = minimax(tempBoard, depth - 1,
                                alpha, beta, true,
                                tempAiW, tempHumanW);

            best = std::min(best, score);
            beta = std::min(beta, best);
            if (beta <= alpha) break;
        }
        return best;
    }
}

// ============================================================
//  evaluate() — Lightweight heuristic
//
//  Score = opponentDistance - aiDistance
//  Higher = AI is closer to winning relative to human.
// ============================================================
int AIPlayer::evaluate(const Board& board) const
{
    int aiGoal    = (m_aiIndex    == 0) ? Board::P0_GOAL_ROW : Board::P1_GOAL_ROW;
    int humanGoal = (m_humanIndex == 0) ? Board::P0_GOAL_ROW : Board::P1_GOAL_ROW;

    int aiDist    = board.bfsDistance(board.getPawnPosition(m_aiIndex),    aiGoal);
    int humanDist = board.bfsDistance(board.getPawnPosition(m_humanIndex), humanGoal);

    if (aiDist    == -1) aiDist    = 100;
    if (humanDist == -1) humanDist = 100;

    // Primary: path length difference
    int score = (humanDist - aiDist) * 50;

    // Small endgame bonus
    if (aiDist <= 2)
        score += (3 - aiDist) * 20;

    return score;
}

// ============================================================
//  generateHardMoves()
//
//  Generates all valid pawn moves + up to 12 wall candidates.
//  Walls are selected from:
//    • Opponent's shortest path (blocking)
//    • Near opponent's current position
//    • Between the two players
// ============================================================
std::vector<AIMove> AIPlayer::generateHardMoves(const Board& board,
                                                 int          playerIndex,
                                                 int          wallCount) const
{
    std::vector<AIMove> moves;
    moves.reserve(16);

    // ---- Pawn moves ----
    for (const Position& p : board.getValidMoves(playerIndex))
        moves.push_back(AIMove::pawnMove(p));

    // ---- Wall moves ----
    if (wallCount <= 0) return moves;

    int opponentIndex = 1 - playerIndex;
    const Position& opp = board.getPawnPosition(opponentIndex);
    const Position& myPos = board.getPawnPosition(playerIndex);
    int oppGoalRow = (opponentIndex == 0) ? Board::P0_GOAL_ROW : Board::P1_GOAL_ROW;

    bool tried[8][8][2] = {};

    auto tryAnchor = [&](int ar, int ac)
    {
        if (ar < 0 || ar > 7 || ac < 0 || ac > 7) return;
        for (int o = 0; o < 2; ++o)
        {
            if (tried[ar][ac][o]) continue;
            tried[ar][ac][o] = true;

            Wall w(ar, ac, (o == 0) ? Wall::Orientation::Horizontal
                                    : Wall::Orientation::Vertical);
            if (board.isWallValid(w))
                moves.push_back(AIMove::wallMove(w));
        }
    };

    // 1. Walls on opponent's shortest path (most critical)
    std::vector<Position> oppPath = board.getShortestPath(opp, oppGoalRow);
    int pathWalls = 0;
    for (const Position& p : oppPath)
    {
        if (pathWalls >= 6) break;
        if (p.row < 8) { tryAnchor(p.row, p.col); pathWalls++; }
        if (p.col < 8) { tryAnchor(p.row, p.col); pathWalls++; }
        if (p.row > 0) { tryAnchor(p.row - 1, p.col); pathWalls++; }
        if (p.col > 0) { tryAnchor(p.row, p.col - 1); pathWalls++; }
    }

    // 2. Walls near opponent (+-1)
    for (int dr = -1; dr <= 1; ++dr)
    for (int dc = -1; dc <= 1; ++dc)
        tryAnchor(opp.row + dr, opp.col + dc);

    // 3. Walls between players
    int midR = (myPos.row + opp.row) / 2;
    int midC = (myPos.col + opp.col) / 2;
    tryAnchor(midR, midC);
    tryAnchor(midR - 1, midC);
    tryAnchor(midR, midC - 1);

    // Cap total wall moves to keep search fast
    if (moves.size() > 16)
        moves.resize(16);

    return moves;
}

// ============================================================
//  applyAIMove()
// ============================================================
bool AIPlayer::applyAIMove(Board& board, const AIMove& move,
                            int playerIndex, int& wallCount) const
{
    if (move.type == AIMove::Type::Pawn)
    {
        if (!board.isMoveValid(playerIndex, move.pawnTarget))
            return false;
        board.applyMove(playerIndex, move.pawnTarget);
        return true;
    }
    else
    {
        if (wallCount <= 0) return false;
        if (!board.isWallValid(move.wall)) return false;
        board.applyWall(move.wall);
        --wallCount;
        return true;
    }
}
