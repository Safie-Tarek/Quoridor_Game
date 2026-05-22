// ============================================================
//  Board.cpp
//  Implementation of the Board class.
//  See Board.h for full design documentation.
// ============================================================

#include "Board.h"
#include <queue>
#include <iostream>
#include <cmath>        // std::abs (floating-point)
#include <cstdlib>      // std::abs (integer overloads)
#include <algorithm>

// ============================================================
//  Constructor
// ============================================================
Board::Board()
{
    reset();
}

// ============================================================
//  reset()
// ============================================================
void Board::reset()
{
    // Clear every wall flag.
    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c)
        {
            m_wallSouth[r][c] = false;
            m_wallEast [r][c] = false;
        }

    // Place pawns at their starting positions.
    m_pawnPos[0] = Position(P0_START_ROW, P0_START_COL);
    m_pawnPos[1] = Position(P1_START_ROW, P1_START_COL);

    m_placedWalls.clear();
}

// ============================================================
//  isMoveValid()
//
//  Delegates to isSimpleMoveValid() for plain one-step moves
//  and isJumpMoveValid() for two-step moves (jumps).
// ============================================================
bool Board::isMoveValid(int playerIndex, const Position& target) const
{
    if (playerIndex < 0 || playerIndex >= NUM_PLAYERS) return false;
    if (!target.isValid())                              return false;

    const Position& from = m_pawnPos[playerIndex];

    int dr = target.row - from.row;
    int dc = target.col - from.col;
    int dist = std::abs(dr) + std::abs(dc);

    if (dist == 1)
    {
        // Plain orthogonal step.
        return isSimpleMoveValid(from, target);
    }
    else if (dist == 2)
    {
        // Could be a straight jump (dr or dc == 2) or a
        // diagonal jump (|dr|==1 && |dc|==1).
        return isJumpMoveValid(playerIndex, from, target);
    }

    return false;   // dist == 0 (staying still) or > 2
}

// ============================================================
//  applyMove()
// ============================================================
void Board::applyMove(int playerIndex, const Position& target)
{
    m_pawnPos[playerIndex] = target;
}

// ============================================================
//  getValidMoves()
//
//  Tries every position in the 9x9 grid that is at Manhattan
//  distance 1 or 2 from the current pawn.  Using isMoveValid()
//  as the filter keeps the logic in one place.
//
//  We only generate candidates at distance 1 or 2 to avoid
//  checking all 81 cells every call.
// ============================================================
std::vector<Position> Board::getValidMoves(int playerIndex) const
{
    std::vector<Position> moves;
    if (playerIndex < 0 || playerIndex >= NUM_PLAYERS) return moves;

    const Position& from = m_pawnPos[playerIndex];

    // All positions reachable in 1 or 2 steps (candidates only -
    // isMoveValid will filter the illegal ones).
    for (int dr = -2; dr <= 2; ++dr)
    {
        for (int dc = -2; dc <= 2; ++dc)
        {
            int dist = std::abs(dr) + std::abs(dc);
            if (dist < 1 || dist > 2) continue;

            Position candidate(from.row + dr, from.col + dc);
            if (isMoveValid(playerIndex, candidate))
                moves.push_back(candidate);
        }
    }

    return moves;
}

// ============================================================
//  isWallValid()
// ============================================================
bool Board::isWallValid(const Wall& wall) const
{
    // 1. Anchor within legal range.
    if (!wall.isValid()) return false;

    // 2. Check against every already-placed wall.
    for (const Wall& existing : m_placedWalls)
    {
        if (wall.overlaps(existing)) return false;
        if (wall.crosses(existing))  return false;
    }

    // 3. BFS on a temporary copy - never touches the live board.
    Board temp = *this;
    temp.writeWallToArrays(wall);

    // Both players must still have a path to their goal.
    if (temp.bfsDistance(m_pawnPos[0], P0_GOAL_ROW) == -1) return false;
    if (temp.bfsDistance(m_pawnPos[1], P1_GOAL_ROW) == -1) return false;

    return true;
}

// ============================================================
//  applyWall()
// ============================================================
void Board::applyWall(const Wall& wall)
{
    writeWallToArrays(wall);
    m_placedWalls.push_back(wall);
}

// ============================================================
//  State queries
// ============================================================
Position Board::getPawnPosition(int playerIndex) const
{
    if (playerIndex < 0 || playerIndex >= NUM_PLAYERS)
        return Position(0, 0);
    return m_pawnPos[playerIndex];
}

const std::vector<Wall>& Board::getPlacedWalls() const
{
    return m_placedWalls;
}

bool Board::hasPlayerWon(int playerIndex) const
{
    if (playerIndex < 0 || playerIndex >= NUM_PLAYERS) return false;
    int goalRow = (playerIndex == 0) ? P0_GOAL_ROW : P1_GOAL_ROW;
    return m_pawnPos[playerIndex].row == goalRow;
}

// ============================================================
//  isEdgeBlocked()
//
//  Direction -> array lookup table:
//
//    Moving South (+1, 0):  check wallSouth[r][c]
//    Moving North (-1, 0):  check wallSouth[r-1][c]
//                           (the cell above owns its south edge)
//    Moving East  (0, +1):  check wallEast[r][c]
//    Moving West  (0, -1):  check wallEast[r][c-1]
//                           (the cell to the left owns its east edge)
// ============================================================
bool Board::isEdgeBlocked(const Position& from, int dr, int dc) const
{
    int r = from.row;
    int c = from.col;

    if (dr == +1 && dc == 0)                         // South
        return m_wallSouth[r][c];

    if (dr == -1 && dc == 0)                         // North
        return (r > 0) && m_wallSouth[r - 1][c];

    if (dr == 0 && dc == +1)                         // East
        return m_wallEast[r][c];

    if (dr == 0 && dc == -1)                         // West
        return (c > 0) && m_wallEast[r][c - 1];

    return true;   // unknown direction -> treat as blocked
}

// ============================================================
//  bfsDistance()
//
//  Standard BFS from 'start' to any cell in 'goalRow'.
//  Returns the number of moves in the shortest path, or -1
//  if no path exists.
//
//  Uses getPassableNeighbors() which respects all wall flags
//  on THIS board object (so it works on temporary copies too).
// ============================================================
int Board::bfsDistance(const Position& start, int goalRow) const
{
    // visited[r][c] = true once cell (r,c) is enqueued.
    bool visited[SIZE][SIZE] = {};

    // Queue stores {position, distance} pairs.
    struct Node { Position pos; int dist; };
    std::queue<Node> frontier;

    frontier.push({ start, 0 });
    visited[start.row][start.col] = true;

    while (!frontier.empty())
    {
        Node node    = frontier.front();
        frontier.pop();
        Position current = node.pos;
        int      dist    = node.dist;

        // Goal reached.
        if (current.row == goalRow)
            return dist;

        for (const Position& neighbor : getPassableNeighbors(current))
        {
            if (!visited[neighbor.row][neighbor.col])
            {
                visited[neighbor.row][neighbor.col] = true;
                frontier.push({ neighbor, dist + 1 });
            }
        }
    }

    return -1;   // no path found
}

// ============================================================
//  getShortestPath()
//
//  BFS with parent tracking to reconstruct the shortest path
//  from 'start' to any cell in 'goalRow'.
//  Returns empty vector if no path exists.
// ============================================================
std::vector<Position> Board::getShortestPath(const Position& start, int goalRow) const
{
    bool visited[SIZE][SIZE] = {};
    Position parent[SIZE][SIZE];

    struct Node { Position pos; int dist; };
    std::queue<Node> frontier;

    frontier.push({ start, 0 });
    visited[start.row][start.col] = true;
    parent[start.row][start.col] = Position(-1, -1);

    Position goalPos(-1, -1);

    while (!frontier.empty())
    {
        Node node = frontier.front();
        frontier.pop();
        Position current = node.pos;

        if (current.row == goalRow)
        {
            goalPos = current;
            break;
        }

        for (const Position& neighbor : getPassableNeighbors(current))
        {
            if (!visited[neighbor.row][neighbor.col])
            {
                visited[neighbor.row][neighbor.col] = true;
                parent[neighbor.row][neighbor.col] = current;
                frontier.push({ neighbor, node.dist + 1 });
            }
        }
    }

    if (goalPos.row == -1)
        return {};  // No path found

    // Reconstruct path
    std::vector<Position> path;
    Position at = goalPos;
    while (at.row != -1)
    {
        path.push_back(at);
        at = parent[at.row][at.col];
    }

    // Reverse to get start -> goal order
    std::reverse(path.begin(), path.end());
    return path;
}


// ============================================================
//  printToConsole()
//  Simple ASCII render for console testing.
// ============================================================
void Board::printToConsole() const
{
    std::cout << "\n";
    for (int r = 0; r < SIZE; ++r)
    {
        // Cell row.
        std::cout << "  ";
        for (int c = 0; c < SIZE; ++c)
        {
            char ch = '.';
            if (m_pawnPos[0] == Position(r, c)) ch = '0';
            if (m_pawnPos[1] == Position(r, c)) ch = '1';
            std::cout << '[' << ch << ']';
            if (c < SIZE - 1)
                std::cout << (m_wallEast[r][c] ? '|' : ' ');
        }
        std::cout << '\n';

        // South-wall row.
        if (r < SIZE - 1)
        {
            std::cout << "  ";
            for (int c = 0; c < SIZE; ++c)
            {
                std::cout << (m_wallSouth[r][c] ? "===" : "   ");
                if (c < SIZE - 1) std::cout << ' ';
            }
            std::cout << '\n';
        }
    }
    std::cout << '\n';
}

// ============================================================
//  writeWallToArrays()  [private]
// ============================================================
void Board::writeWallToArrays(const Wall& wall)
{
    int r = wall.getAnchorRow();
    int c = wall.getAnchorCol();

    if (wall.getOrientation() == Wall::Orientation::Horizontal)
    {
        m_wallSouth[r][c]     = true;
        m_wallSouth[r][c + 1] = true;
    }
    else
    {
        m_wallEast[r][c]     = true;
        m_wallEast[r + 1][c] = true;
    }
}

// ============================================================
//  getPassableNeighbors()  [private]
//
//  Returns all cells reachable in one orthogonal step from
//  'pos', respecting walls but ignoring pawns.
//  This is the BFS adjacency function.
// ============================================================
std::vector<Position> Board::getPassableNeighbors(const Position& pos) const
{
    std::vector<Position> neighbors;

    for (const auto& d : DIRECTIONS)
    {
        int dr = d[0], dc = d[1];
        Position next(pos.row + dr, pos.col + dc);

        if (!next.isValid())          continue;
        if (isEdgeBlocked(pos, dr, dc)) continue;

        neighbors.push_back(next);
    }

    return neighbors;
}

// ============================================================
//  isSimpleMoveValid()  [private]
//
//  A plain one-step orthogonal move is valid when:
//    * No wall blocks the shared edge
//    * The target cell is not occupied by the OTHER pawn
//      (can't step into their cell - must jump instead)
// ============================================================
bool Board::isSimpleMoveValid(const Position& from, const Position& to) const
{
    int dr = to.row - from.row;
    int dc = to.col - from.col;

    // Edge blocked by a wall?
    if (isEdgeBlocked(from, dr, dc)) return false;

    // Target cell occupied by any pawn?
    for (int i = 0; i < NUM_PLAYERS; ++i)
        if (m_pawnPos[i] == to) return false;

    return true;
}

// ============================================================
//  isJumpMoveValid()  [private]
//
//  Jump rules (official Quoridor):
//
//  STRAIGHT JUMP:
//    The adjacent cell (one step in direction D) contains the
//    opponent's pawn.  The player may jump OVER them (two steps
//    in direction D) provided:
//      * No wall blocks the edge between (from) and (opponent)
//      * No wall blocks the edge between (opponent) and (landing)
//      * The landing cell is empty
//
//  DIAGONAL JUMP (blocked straight jump):
//    If the straight jump is blocked by a wall behind the opponent,
//    the player may instead jump to the left or right of the
//    opponent (i.e. diagonally relative to the original direction).
//    This is only legal when:
//      * No wall blocks the edge between (from) and (opponent)
//      * The diagonal landing cell is on the board and empty
//      * No wall blocks the edge between (opponent) and (diagonal cell)
// ============================================================
bool Board::isJumpMoveValid(int playerIndex,
                             const Position& from,
                             const Position& to) const
{
    int opponentIndex = 1 - playerIndex;
    const Position& opponentPos = m_pawnPos[opponentIndex];

    int dr = to.row - from.row;
    int dc = to.col - from.col;

    // ---- STRAIGHT JUMP: target is two steps away on one axis ----
    // e.g. from (5,4) to (3,4) - dr=-2, dc=0
    if ((std::abs(dr) == 2 && dc == 0) ||
        (dr == 0 && std::abs(dc) == 2))
    {
        // The cell halfway between from and to must be the opponent.
        Position midpoint(from.row + dr / 2, from.col + dc / 2);
        if (midpoint != opponentPos) return false;

        // Edge from 'from' to opponent must be open.
        if (isEdgeBlocked(from, dr / 2, dc / 2)) return false;

        // Edge from opponent to landing must be open.
        if (isEdgeBlocked(opponentPos, dr / 2, dc / 2)) return false;

        // Landing cell must be empty.
        for (int i = 0; i < NUM_PLAYERS; ++i)
            if (m_pawnPos[i] == to) return false;

        return true;
    }

    // ---- DIAGONAL JUMP: target is one step diagonally ----
    // e.g. from (5,4), opponent at (4,4), diagonal landing (4,3) or (4,5)
    // |dr|==1 && |dc|==1
    if (std::abs(dr) == 1 && std::abs(dc) == 1)
    {
        // Which direction is "toward the opponent"?
        // One of dr or dc will be the primary direction; the other is
        // the sideways offset.  We try both axis combinations.

        // Case A: the opponent is directly above/below (same col as 'from')
        //         and we jump diagonally sideways.
        Position opponentViaRow(from.row + dr, from.col);
        if (opponentViaRow == opponentPos)
        {
            // Edge from 'from' to opponent must be open.
            if (isEdgeBlocked(from, dr, 0)) return false;
            // Straight jump must be blocked (wall behind opponent).
            if (!isEdgeBlocked(opponentPos, dr, 0)) return false;
            // Edge from opponent to diagonal landing must be open.
            if (isEdgeBlocked(opponentPos, 0, dc)) return false;
            // Landing must be in bounds and empty.
            if (!to.isValid()) return false;
            for (int i = 0; i < NUM_PLAYERS; ++i)
                if (m_pawnPos[i] == to) return false;
            return true;
        }

        // Case B: the opponent is directly left/right (same row as 'from')
        //         and we jump diagonally up/down.
        Position opponentViaCol(from.row, from.col + dc);
        if (opponentViaCol == opponentPos)
        {
            if (isEdgeBlocked(from, 0, dc))           return false;
            if (!isEdgeBlocked(opponentPos, 0, dc))   return false;
            if (isEdgeBlocked(opponentPos, dr, 0))    return false;
            if (!to.isValid())                         return false;
            for (int i = 0; i < NUM_PLAYERS; ++i)
                if (m_pawnPos[i] == to) return false;
            return true;
        }
    }

    return false;
}
