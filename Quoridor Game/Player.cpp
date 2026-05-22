// ============================================================
//  Player.cpp
// ============================================================

#include "Player.h"
#include <iostream>

Player::Player()
    : m_name("Player")
    , m_position(0, 0)
    , m_startPos(0, 0)
    , m_wallCount(STARTING_WALLS)
    , m_goalRow(0)
    , m_playerIndex(0)
{}

Player::Player(const std::string& name,
               const Position&    startPos,
               int                goalRow,
               int                playerIndex)
    : m_name(name)
    , m_position(startPos)
    , m_startPos(startPos)
    , m_wallCount(STARTING_WALLS)
    , m_goalRow(goalRow)
    , m_playerIndex(playerIndex)
{}

// ---- Getters ------------------------------------------------

const std::string& Player::getName()        const { return m_name;        }
const Position&    Player::getPosition()    const { return m_position;    }
int                Player::getWallCount()   const { return m_wallCount;   }
int                Player::getGoalRow()     const { return m_goalRow;     }
int                Player::getPlayerIndex() const { return m_playerIndex; }

// ---- State updates ------------------------------------------

void Player::setPosition(const Position& newPos)
{
    m_position = newPos;
}

void Player::useWall()
{
    if (m_wallCount > 0)
        --m_wallCount;
}

// ---- Queries ------------------------------------------------

bool Player::hasWallsRemaining() const
{
    return m_wallCount > 0;
}

bool Player::hasWon() const
{
    return m_position.row == m_goalRow;
}

void Player::reset()
{
    m_position  = m_startPos;
    m_wallCount = STARTING_WALLS;
}

void Player::printStatus() const
{
    std::cout << "[Player " << m_playerIndex << " | " << m_name << "]"
              << "  pos=(" << m_position.row << "," << m_position.col << ")"
              << "  walls=" << m_wallCount
              << "  won=" << (hasWon() ? "YES" : "no")
              << '\n';
}
