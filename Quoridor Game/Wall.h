#pragma once

// ============================================================
//  Wall.h
//
//  Represents a single wall piece placed on the Quoridor board.
//
//  Physical model
//  --------------
//  A wall covers exactly TWO consecutive cell-edges and is
//  described by:
//    1. An ANCHOR cell  (the top-left cell the wall touches)
//    2. An ORIENTATION  (which direction the wall extends)
//
//  Horizontal wall at anchor (r, c):
//    Blocks the SOUTH edge of cells (r,c) and (r, c+1).
//    A pawn cannot cross from row r down to row r+1
//    through columns c or c+1.
//
//        col c    col c+1
//         [ ] ════ [ ]       row r
//        ────────────────    ← wall on this line
//         [ ]      [ ]       row r+1
//
//  Vertical wall at anchor (r, c):
//    Blocks the EAST edge of cells (r,c) and (r+1, c).
//    A pawn cannot cross from col c right to col c+1
//    through rows r or r+1.
//
//        col c  col c+1
//         [ ] ║  [ ]         row r
//         [ ] ║  [ ]         row r+1
//              ↑
//           wall here
//
//  Anchor constraint:
//    row and col must both be in [0..7].
//    A wall at row 8 would extend to row 9 — off the board.
//
//  Refactor notes (vs original):
//    • Renamed m_row/m_col → m_anchorRow/m_anchorCol for clarity
//    • Moved Orientation enum inside Wall as a nested type so
//      usage reads Wall::Orientation::Horizontal — self-contained
//    • Simplified overlaps() with a cleaner cell-pair comparison
//    • Added getAnchorPosition() convenience getter
// ============================================================

#include "Position.h"

class Wall
{
public:

    // ---- Nested enum ----------------------------------------

    // Scoped enum so usage is always  Wall::Orientation::Horizontal.
    // This makes code at call sites self-documenting.
    enum class Orientation
    {
        Horizontal,   // wall runs left-right, blocks N/S movement
        Vertical      // wall runs top-bottom, blocks E/W movement
    };

    // ---- Constants ------------------------------------------

    // Maximum valid anchor coordinate (0-indexed, wall spans 2 cells)
    static constexpr int MAX_ANCHOR = 7;

    // ---- Constructors ---------------------------------------

    // Default: horizontal wall at (0,0).
    // Needed so Wall can live in std::vector without explicit init.
    Wall();

    // Normal construction.
    //   anchorRow, anchorCol — top-left cell of the wall [0..7]
    //   orientation          — Horizontal or Vertical
    Wall(int anchorRow, int anchorCol, Orientation orientation);

    // ---- Getters --------------------------------------------

    int         getAnchorRow()    const;
    int         getAnchorCol()    const;
    Orientation getOrientation()  const;

    // Returns the anchor as a Position struct — convenient for
    // code that already works with Position objects.
    Position    getAnchorPosition() const;

    // ---- Validation -----------------------------------------

    // Returns true when the anchor lies in [0..7] x [0..7],
    // meaning the wall fits entirely on the board.
    // Does NOT check for overlaps with other walls.
    bool isValid() const;

    // ---- Conflict detection ---------------------------------

    // overlaps()
    //   Returns true if this wall shares at least one cell-edge
    //   with 'other'.  Two walls cannot share any edge.
    //
    //   How it works:
    //     Each wall claims two specific cell-edges.
    //     We encode every edge as a unique integer and check
    //     whether the two sets of edge-IDs intersect.
    //
    //     South-edge ID of cell (r,c)  →  r * 9 + c          [0..71]
    //     East-edge  ID of cell (r,c)  →  72 + r * 8 + c     [72..143]
    bool overlaps(const Wall& other) const;

    // crosses()
    //   Returns true if this wall and 'other' form a "+" shape.
    //   This only happens when a Horizontal and a Vertical wall
    //   share exactly the same anchor cell.
    //
    //   Visual:
    //     H-wall at (r,c) and V-wall at (r,c) intersect at the
    //     corner point shared by cells (r,c),(r,c+1),(r+1,c),(r+1,c+1).
    bool crosses(const Wall& other) const;

    // ---- Equality -------------------------------------------

    // Two walls are equal when they have identical anchor and orientation.
    bool operator==(const Wall& other) const;

private:

    // The top-left cell the wall touches.
    // Named "anchor" rather than just row/col to make the
    // concept clear: it is the reference point, not the whole wall.
    int         m_anchorRow;
    int         m_anchorCol;

    Orientation m_orientation;
};
