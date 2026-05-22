// ============================================================
//  Wall.cpp
//  Implementation of the Wall class.  See Wall.h for the
//  physical model and design documentation.
// ============================================================

#include "Wall.h"

// ---- Constructors -------------------------------------------

Wall::Wall()
    : m_anchorRow(0)
    , m_anchorCol(0)
    , m_orientation(Orientation::Horizontal)
{}

Wall::Wall(int anchorRow, int anchorCol, Orientation orientation)
    : m_anchorRow(anchorRow)
    , m_anchorCol(anchorCol)
    , m_orientation(orientation)
{}

// ---- Getters ------------------------------------------------

int Wall::getAnchorRow() const { return m_anchorRow; }
int Wall::getAnchorCol() const { return m_anchorCol; }
Wall::Orientation Wall::getOrientation() const { return m_orientation; }

Position Wall::getAnchorPosition() const
{
    return Position(m_anchorRow, m_anchorCol);
}

// ---- Validation ---------------------------------------------

bool Wall::isValid() const
{
    return m_anchorRow >= 0 && m_anchorRow <= MAX_ANCHOR
        && m_anchorCol >= 0 && m_anchorCol <= MAX_ANCHOR;
}

// ---- Conflict detection -------------------------------------

// overlaps()
//
//  Edge encoding (see Wall.h for full explanation):
//
//  A Horizontal wall at (r, c) claims:
//    southEdgeId(r, c)   = r * 9 + c
//    southEdgeId(r, c+1) = r * 9 + (c+1)
//
//  A Vertical wall at (r, c) claims:
//    eastEdgeId(r,   c)  = 72 + r     * 8 + c
//    eastEdgeId(r+1, c)  = 72 + (r+1) * 8 + c
//
//  If any edge-ID from THIS wall matches any edge-ID from OTHER,
//  the walls share an edge and cannot coexist.
bool Wall::overlaps(const Wall& other) const
{
    // Build the two edge-IDs for THIS wall.
    int myEdgeA, myEdgeB;
    if (m_orientation == Orientation::Horizontal)
    {
        myEdgeA = m_anchorRow * 9 + m_anchorCol;
        myEdgeB = m_anchorRow * 9 + (m_anchorCol + 1);
    }
    else
    {
        myEdgeA = 72 + m_anchorRow       * 8 + m_anchorCol;
        myEdgeB = 72 + (m_anchorRow + 1) * 8 + m_anchorCol;
    }

    // Build the two edge-IDs for the OTHER wall.
    int otherEdgeA, otherEdgeB;
    if (other.m_orientation == Orientation::Horizontal)
    {
        otherEdgeA = other.m_anchorRow * 9 + other.m_anchorCol;
        otherEdgeB = other.m_anchorRow * 9 + (other.m_anchorCol + 1);
    }
    else
    {
        otherEdgeA = 72 + other.m_anchorRow       * 8 + other.m_anchorCol;
        otherEdgeB = 72 + (other.m_anchorRow + 1) * 8 + other.m_anchorCol;
    }

    // Any shared edge-ID means overlap.
    return (myEdgeA == otherEdgeA || myEdgeA == otherEdgeB ||
            myEdgeB == otherEdgeA || myEdgeB == otherEdgeB);
}

// crosses()
//
//  Two walls of different orientations cross when they share
//  the same anchor cell.  Same-orientation walls can never cross.
bool Wall::crosses(const Wall& other) const
{
    if (m_orientation == other.m_orientation)
        return false;   // same orientation → can't cross

    return (m_anchorRow == other.m_anchorRow &&
            m_anchorCol == other.m_anchorCol);
}

// ---- Equality -----------------------------------------------

bool Wall::operator==(const Wall& other) const
{
    return m_anchorRow    == other.m_anchorRow
        && m_anchorCol    == other.m_anchorCol
        && m_orientation  == other.m_orientation;
}
