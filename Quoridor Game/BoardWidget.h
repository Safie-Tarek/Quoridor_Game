#pragma once

// ============================================================
//  BoardWidget.h
//
//  A Qt widget responsible for:
//    * Rendering the 9x9 Quoridor board
//    * Drawing pawns, placed walls, valid-move highlights
//    * Drawing a ghost (preview) wall while the mouse hovers
//    * Translating mouse events into Game API calls
//
//  Architecture rule:
//    BoardWidget READS game state through the Game* it holds.
//    All WRITES go through Game methods.
//    BoardWidget contains zero game rules.
//
//  NEW (v2.0):
//    • Added refreshBoard() slot for external repaint requests
//    • Added setShowValidMoves() to toggle highlight overlay
//    • Added right-click support for wall rotation
// ============================================================

#include <QWidget>
#include <QPoint>
#include "Game.h"
#include "Wall.h"

class BoardWidget : public QWidget
{
    Q_OBJECT

public:

    static constexpr int CELL_SIZE   = 72;
    static constexpr int WALL_GAP    = 8;
    static constexpr int BOARD_MARGIN = 30;
    static constexpr int CELL_STEP = CELL_SIZE + WALL_GAP;

    explicit BoardWidget(Game* game, QWidget* parent = nullptr);

    void setGame(Game* game);

    // NEW: Toggle whether valid-move highlights are shown.
    void setShowValidMoves(bool show);

    // NEW: Clear wall preview (called when switching to move mode).
    void clearWallPreview();

signals:

    // Emitted after any successful action so GameWindow can
    // update its status label and other UI elements.
    void gameStateChanged();

    // NEW: Emitted when a pawn move is requested (for MainWindow architecture).
    void pawnMoveRequested(const Position& target);

    // NEW: Emitted when a wall placement is requested (for MainWindow architecture).
    void wallPlaceRequested(const Wall& wall);

public slots:

    // Called by GameWindow when the W key is pressed to toggle
    // between pawn-move mode and wall-placement mode.
    void toggleInputMode();

    // Called by GameWindow when the R key is pressed to rotate
    // the wall orientation during wall-placement mode.
    void rotateWallOrientation();

    // NEW: Explicit refresh request from parent window.
    void refreshBoard();

protected:

    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:

    Game* m_game;

    Wall::Orientation m_wallOrientation;
    Wall              m_previewWall;
    bool              m_showPreview;

    // NEW: Whether to draw valid-move highlights.
    bool              m_showValidMoves;

    Position pixelToCell(const QPoint& pixel) const;
    Position pixelToWallAnchor(const QPoint& pixel) const;
    QPoint cellTopLeft(int row, int col) const;

    void drawGrid(QPainter& painter)       const;
    void drawValidMoveHighlights(QPainter& painter) const;
    void drawWalls(QPainter& painter)      const;
    void drawWallPreview(QPainter& painter) const;
    void drawPawns(QPainter& painter)      const;
    void drawGoalRows(QPainter& painter)   const;
};
