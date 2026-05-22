#pragma once

// ============================================================
//  Position.h
//
//  A lightweight struct that represents a single cell on the
//  9x9 Quoridor board using (row, col) coordinates.
//
//  Coordinate system:
//    row 0 = TOP  of the board  (Player 1's starting side)
//    row 8 = BOTTOM             (Player 0's starting side)
//    col 0 = LEFT edge
//    col 8 = RIGHT edge
//
//  Why a struct and not a class?
//    Position is pure data — it has no invariants to protect
//    and no behaviour beyond comparison.  A struct with all
//    public members is the clearest way to express this.
//
//  Refactor notes (vs original):
//    • Added encode() helper for use as a BFS visited-array key
//    • Operator+ added to allow  pos + delta  arithmetic
//    • Kept deliberately simple — no unnecessary methods
// ============================================================

struct Position
{
    int row { 0 };   // Vertical coordinate   — valid range [0..8]
    int col { 0 };   // Horizontal coordinate — valid range [0..8]

    // ---- Constructors ----------------------------------------

    // Default: produces (0, 0) so Position can be declared
    // without an immediate value (e.g. inside other classes).
    Position() = default;

    // Normal construction: Position p(3, 4);
    Position(int r, int c) : row(r), col(c) {}

    // ---- Validity check --------------------------------------

    // Returns true when this position lies inside the 9x9 grid.
    // Call this before any board lookup to avoid out-of-bounds.
    bool isValid() const
    {
        return row >= 0 && row <= 8
            && col >= 0 && col <= 8;
    }

    // ---- Arithmetic ------------------------------------------

    // Adds another Position as a delta, returning a new Position.
    // Typical usage:  Position next = current + Position(-1, 0);
    Position operator+(const Position& delta) const
    {
        return Position(row + delta.row, col + delta.col);
    }

    // ---- Comparison ------------------------------------------

    bool operator==(const Position& other) const
    {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Position& other) const
    {
        return !(*this == other);
    }

    // ---- BFS helper ------------------------------------------

    // Encodes this position as a single integer for use as an
    // array index in BFS visited grids.
    //   encode() = row * 9 + col   →   unique value in [0..80]
    int encode() const
    {
        return row * 9 + col;
    }
};
