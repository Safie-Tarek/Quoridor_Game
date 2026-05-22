// ============================================================
//  BoardWidget.cpp
//  Rendering and mouse input for the Quoridor board.
//
//  NEW (v2.0):
//    • refreshBoard() slot for explicit repaint
//    • setShowValidMoves() to toggle highlights
//    • Right-click rotates wall orientation
//    • Emits pawnMoveRequested / wallPlaceRequested signals
//      for MainWindow architecture compatibility
// ============================================================

#include "BoardWidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>
#include <QColor>

// ============================================================
//  Constructor
// ============================================================
BoardWidget::BoardWidget(Game* game, QWidget* parent)
    : QWidget(parent)
    , m_game(game)
    , m_wallOrientation(Wall::Orientation::Horizontal)
    , m_previewWall()
    , m_showPreview(false)
    , m_showValidMoves(true)
{
    int boardPx = 9 * CELL_SIZE + 8 * WALL_GAP + 2 * BOARD_MARGIN;
    setFixedSize(boardPx, boardPx);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

// ============================================================
//  Slots
// ============================================================
void BoardWidget::toggleInputMode()
{
    Game::InputMode newMode =
        (m_game->getInputMode() == Game::InputMode::MovePawn)
        ? Game::InputMode::PlaceWall
        : Game::InputMode::MovePawn;

    m_game->setInputMode(newMode);
    m_showPreview = false;
    update();
    emit gameStateChanged();
}

void BoardWidget::rotateWallOrientation()
{
    m_wallOrientation =
        (m_wallOrientation == Wall::Orientation::Horizontal)
        ? Wall::Orientation::Vertical
        : Wall::Orientation::Horizontal;

    m_showPreview = false;
    update();
}

void BoardWidget::refreshBoard()
{
    update();
}

void BoardWidget::setShowValidMoves(bool show)
{
    m_showValidMoves = show;
    update();
}

void BoardWidget::clearWallPreview()
{
    m_showPreview = false;
    update();
}

// ============================================================
//  paintEvent()
// ============================================================
void BoardWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(245, 230, 200));

    drawGoalRows(painter);
    drawGrid(painter);
    drawValidMoveHighlights(painter);
    drawWalls(painter);
    drawWallPreview(painter);
    drawPawns(painter);
}

// ============================================================
//  mouseMoveEvent()
// ============================================================
void BoardWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_game->getInputMode() != Game::InputMode::PlaceWall ||
        m_game->isGameOver())
    {
        if (m_showPreview) {
            m_showPreview = false;
            update();
        }
        return;
    }

    Position anchor = pixelToWallAnchor(event->pos());
    if (anchor.isValid() && anchor.row <= 7 && anchor.col <= 7)
    {
        Wall candidate(anchor.row, anchor.col, m_wallOrientation);
        m_previewWall = candidate;
        m_showPreview = true;
    }
    else
    {
        m_showPreview = false;
    }

    update();
}

// ============================================================
//  mousePressEvent()
// ============================================================
void BoardWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_game->isGameOver()) return;

    // Right-click rotates wall orientation in wall mode
    if (event->button() == Qt::RightButton)
    {
        if (m_game->getInputMode() == Game::InputMode::PlaceWall)
        {
            rotateWallOrientation();
        }
        return;
    }

    if (event->button() != Qt::LeftButton) return;

    if (m_game->getInputMode() == Game::InputMode::MovePawn)
    {
        Position target = pixelToCell(event->pos());
        if (!target.isValid()) return;

        // Emit signal for MainWindow architecture
        emit pawnMoveRequested(target);

        // Also call directly for GameWindow architecture
        m_game->moveCurrentPlayer(target);
    }
    else // PlaceWall
    {
        if (!m_showPreview) return;

        // Emit signal for MainWindow architecture
        emit wallPlaceRequested(m_previewWall);

        // Also call directly for GameWindow architecture
        m_game->placeWallForCurrentPlayer(m_previewWall);
        m_showPreview = false;
    }

    update();
    emit gameStateChanged();
}

// ============================================================
//  mouseReleaseEvent()
// ============================================================
void BoardWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    // No special handling needed
}

// ============================================================
//  leaveEvent()
// ============================================================
void BoardWidget::leaveEvent(QEvent* /*event*/)
{
    m_showPreview = false;
    update();
}

// ============================================================
//  Drawing helpers
// ============================================================
void BoardWidget::drawGoalRows(QPainter& painter) const
{
    QColor p0GoalColor(173, 216, 230, 120);
    for (int c = 0; c < 9; ++c)
    {
        QPoint tl = cellTopLeft(0, c);
        painter.fillRect(tl.x(), tl.y(), CELL_SIZE, CELL_SIZE, p0GoalColor);
    }

    QColor p1GoalColor(255, 182, 193, 120);
    for (int c = 0; c < 9; ++c)
    {
        QPoint tl = cellTopLeft(8, c);
        painter.fillRect(tl.x(), tl.y(), CELL_SIZE, CELL_SIZE, p1GoalColor);
    }
}

void BoardWidget::drawGrid(QPainter& painter) const
{
    painter.setPen(QPen(QColor(139, 115, 85), 1));
    painter.setBrush(QColor(255, 248, 220));

    for (int r = 0; r < 9; ++r)
    for (int c = 0; c < 9; ++c)
    {
        QPoint tl = cellTopLeft(r, c);
        painter.drawRoundedRect(tl.x(), tl.y(), CELL_SIZE, CELL_SIZE, 4, 4);
    }
}

void BoardWidget::drawValidMoveHighlights(QPainter& painter) const
{
    if (!m_showValidMoves) return;
    if (m_game->getInputMode() != Game::InputMode::MovePawn) return;
    if (m_game->isGameOver()) return;

    QColor highlightColor(144, 238, 144, 180);

    for (const Position& pos : m_game->getValidMovesForCurrentPlayer())
    {
        QPoint tl = cellTopLeft(pos.row, pos.col);
        painter.fillRect(tl.x() + 4, tl.y() + 4,
                         CELL_SIZE - 8, CELL_SIZE - 8,
                         highlightColor);
    }
}

void BoardWidget::drawWalls(QPainter& painter) const
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(80, 50, 20));

    for (const Wall& wall : m_game->getBoard().getPlacedWalls())
    {
        int r = wall.getAnchorRow();
        int c = wall.getAnchorCol();
        QPoint tl = cellTopLeft(r, c);

        if (wall.getOrientation() == Wall::Orientation::Horizontal)
        {
            int x = tl.x();
            int y = tl.y() + CELL_SIZE;
            int w = 2 * CELL_SIZE + WALL_GAP;
            int h = WALL_GAP;
            painter.drawRoundedRect(x, y, w, h, 2, 2);
        }
        else
        {
            int x = tl.x() + CELL_SIZE;
            int y = tl.y();
            int w = WALL_GAP;
            int h = 2 * CELL_SIZE + WALL_GAP;
            painter.drawRoundedRect(x, y, w, h, 2, 2);
        }
    }
}

void BoardWidget::drawWallPreview(QPainter& painter) const
{
    if (!m_showPreview) return;
    if (m_game->getInputMode() != Game::InputMode::PlaceWall) return;

    bool valid = m_game->getBoard().isWallValid(m_previewWall);
    QColor previewColor = valid
        ? QColor(60, 180, 60, 140)
        : QColor(220, 60, 60, 140);

    painter.setPen(Qt::NoPen);
    painter.setBrush(previewColor);

    int r = m_previewWall.getAnchorRow();
    int c = m_previewWall.getAnchorCol();
    QPoint tl = cellTopLeft(r, c);

    if (m_previewWall.getOrientation() == Wall::Orientation::Horizontal)
    {
        int x = tl.x();
        int y = tl.y() + CELL_SIZE;
        int w = 2 * CELL_SIZE + WALL_GAP;
        int h = WALL_GAP;
        painter.drawRoundedRect(x, y, w, h, 2, 2);
    }
    else
    {
        int x = tl.x() + CELL_SIZE;
        int y = tl.y();
        int w = WALL_GAP;
        int h = 2 * CELL_SIZE + WALL_GAP;
        painter.drawRoundedRect(x, y, w, h, 2, 2);
    }
}

void BoardWidget::drawPawns(QPainter& painter) const
{
    const QColor pawnColors[2] = { QColor(30, 100, 200), QColor(200, 40, 40) };
    const QColor textColor(255, 255, 255);
    const QColor turnRingColor(255, 215, 0);   // Gold / yellow

    int currentPlayer = m_game->getCurrentPlayerIndex();

    for (int i = 0; i < 2; ++i)
    {
        const Position& pos = m_game->getBoard().getPawnPosition(i);
        QPoint tl = cellTopLeft(pos.row, pos.col);

        int margin = 6;
        int diameter = CELL_SIZE - 2 * margin;

        // ---- Turn indicator ring (drawn FIRST, behind everything) ----
        if (i == currentPlayer && !m_game->isGameOver())
        {
            int ringMargin = margin - 4;
            int ringDiameter = CELL_SIZE - 2 * ringMargin;
            painter.setPen(Qt::NoPen);
            painter.setBrush(turnRingColor);
            painter.drawEllipse(tl.x() + ringMargin, tl.y() + ringMargin,
                                ringDiameter, ringDiameter);
        }

        // ---- Drop shadow ----
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 60));
        painter.drawEllipse(tl.x() + margin + 2, tl.y() + margin + 2,
                            diameter, diameter);

        // ---- Pawn body ----
        painter.setPen(QPen(Qt::white, 2));
        painter.setBrush(pawnColors[i]);
        painter.drawEllipse(tl.x() + margin, tl.y() + margin,
                            diameter, diameter);

        // ---- Pawn number ----
        painter.setPen(textColor);
        QFont f = painter.font();
        f.setBold(true);
        f.setPointSize(12);
        painter.setFont(f);
        painter.drawText(tl.x(), tl.y(), CELL_SIZE, CELL_SIZE,
                         Qt::AlignCenter,
                         QString::number(i + 1));
    }
}

// ============================================================
//  Coordinate conversion helpers
// ============================================================
QPoint BoardWidget::cellTopLeft(int row, int col) const
{
    int x = BOARD_MARGIN + col * CELL_STEP;
    int y = BOARD_MARGIN + row * CELL_STEP;
    return QPoint(x, y);
}

Position BoardWidget::pixelToCell(const QPoint& pixel) const
{
    int px = pixel.x() - BOARD_MARGIN;
    int py = pixel.y() - BOARD_MARGIN;

    if (px < 0 || py < 0) return Position(-1, -1);

    int col = px / CELL_STEP;
    int row = py / CELL_STEP;

    int localX = px % CELL_STEP;
    int localY = py % CELL_STEP;

    if (localX > CELL_SIZE || localY > CELL_SIZE)
        return Position(-1, -1);

    if (col < 0 || col > 8 || row < 0 || row > 8)
        return Position(-1, -1);

    return Position(row, col);
}

Position BoardWidget::pixelToWallAnchor(const QPoint& pixel) const
{
    int px = pixel.x() - BOARD_MARGIN;
    int py = pixel.y() - BOARD_MARGIN;

    if (px < 0 || py < 0) return Position(-1, -1);

    int col = px / CELL_STEP;
    int row = py / CELL_STEP;

    col = std::min(col, 7);
    row = std::min(row, 7);

    if (col < 0 || row < 0) return Position(-1, -1);

    return Position(row, col);
}

// ============================================================
//  setGame()
// ============================================================
void BoardWidget::setGame(Game* game)
{
    m_game = game;
    m_showPreview = false;
    update();
}
