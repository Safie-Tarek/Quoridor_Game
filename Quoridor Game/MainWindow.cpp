// ============================================================
//  MainWindow.cpp
//
//  NEW (v2.0):
//    • AI Difficulty selector dropdown in toolbar
//    • Undo / Redo buttons with Ctrl+Z / Ctrl+Y shortcuts
//    • Visual enable/disable for undo/redo availability
// ============================================================

#include "MainWindow.h"
#include "BoardWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTimer>
#include <QMessageBox>
#include <QFrame>
#include <QFont>
#include <QSizePolicy>
#include <QString>
#include <QKeyEvent>
#include <QShortcut>
#include <QScreen>
#include <QGuiApplication>

// ============================================================
//  Constructor
// ============================================================
MainWindow::MainWindow(Game::Mode mode, Game::AIDifficulty diff, QWidget* parent)
    : QMainWindow(parent)
    , m_game(std::make_unique<Game>(mode, diff))
    , m_boardWidget(nullptr)
    , m_statusLabel(nullptr)
    , m_wallsLabel(nullptr)
    , m_modeLabel(nullptr)
    , m_modeSelector(nullptr)
    , m_resetButton(nullptr)
    , m_difficultySelector(nullptr)
    , m_difficultyLabel(nullptr)
    , m_movePawnBtn(nullptr)
    , m_placeWallBtn(nullptr)
    , m_highlightBtn(nullptr)
    , m_undoButton(nullptr)
    , m_redoButton(nullptr)
    , m_flashTimer(nullptr)
{
    setWindowTitle("Quoridor");
    setupUI();

    // Sync mode selector to actual game mode WITHOUT triggering onModeChanged
    int modeIndex = (m_game->getMode() == Game::Mode::HumanVsAI) ? 1 : 0;
    m_modeSelector->blockSignals(true);
    m_modeSelector->setCurrentIndex(modeIndex);
    m_modeSelector->blockSignals(false);

    // Sync difficulty selector WITHOUT triggering onDifficultyChanged
    int diffIndex = static_cast<int>(m_game->getAIDifficulty());
    m_difficultySelector->blockSignals(true);
    m_difficultySelector->setCurrentIndex(diffIndex);
    m_difficultySelector->blockSignals(false);

    // Show/hide difficulty based on actual mode
    bool isHvAI = (m_game->getMode() == Game::Mode::HumanVsAI);
    m_difficultyLabel->setVisible(isHvAI);
    m_difficultySelector->setVisible(isHvAI);

    // Center window on screen and size appropriately
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect available = screen->availableGeometry();
    int targetHeight = available.height() - 60;  // Leave margin
    int targetWidth = m_boardWidget->width() + 320;
    resize(targetWidth, targetHeight);

    // Center horizontally, near top vertically
    int x = available.center().x() - targetWidth / 2;
    int y = 30;  // Small top margin
    move(x, y);
    updateStatusLabels();
}

// ============================================================
//  setupUI
// ============================================================
void MainWindow::setupUI()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet("background-color: #2a2018;");

    QVBoxLayout* outerLayout = new QVBoxLayout(central);
    outerLayout->setContentsMargins(12, 12, 12, 12);
    outerLayout->setSpacing(10);

    outerLayout->addWidget(createToolbar());

    m_boardWidget = new BoardWidget(m_game.get(), central);

    QHBoxLayout* boardRow = new QHBoxLayout();
    boardRow->addStretch(1);
    boardRow->addWidget(m_boardWidget);
    boardRow->addStretch(1);
    outerLayout->addLayout(boardRow);

    outerLayout->addWidget(createStatusPanel());

    connect(m_boardWidget, &BoardWidget::pawnMoveRequested,
            this,          &MainWindow::onPawnMoveRequested);
    connect(m_boardWidget, &BoardWidget::wallPlaceRequested,
            this,          &MainWindow::onWallPlaceRequested);

    m_flashTimer = new QTimer(this);
    m_flashTimer->setSingleShot(true);
    connect(m_flashTimer, &QTimer::timeout, this, [this]() {
        m_statusLabel->setStyleSheet("color: #f0e8d0; font-size: 15px; font-weight: bold;");
    });
}

// ============================================================
//  createToolbar
// ============================================================
QWidget* MainWindow::createToolbar()
{
    QWidget* bar = new QWidget();
    bar->setStyleSheet("background-color: #3a2e1c; border-radius: 8px;");

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(12);

    QLabel* titleLabel = new QLabel("QUORIDOR");
    titleLabel->setStyleSheet("color: #d4a84b; font-size: 18px; font-weight: bold; letter-spacing: 3px;");
    layout->addWidget(titleLabel);

    layout->addStretch(1);

    // Valid-move highlight toggle
    m_highlightBtn = new QPushButton("✓ Highlights");
    m_highlightBtn->setCheckable(true);
    m_highlightBtn->setChecked(true);
    m_highlightBtn->setToolTip("Toggle valid-move highlighting");
    m_highlightBtn->setStyleSheet(
        "QPushButton { background:#4a7c4a; color:white; border-radius:5px; "
        "              padding:5px 10px; font-size:12px; }"
        "QPushButton:checked { background:#4a7c4a; }"
        "QPushButton:!checked { background:#5a4a4a; }"
    );
    connect(m_highlightBtn, &QPushButton::toggled,
            this,           &MainWindow::onToggleValidMoves);
    layout->addWidget(m_highlightBtn);

    // Game mode selector
    QLabel* modeLabel2 = new QLabel("Mode:");
    modeLabel2->setStyleSheet("color: #c0b090; font-size: 13px;");
    layout->addWidget(modeLabel2);

    m_modeSelector = new QComboBox();
    m_modeSelector->addItem("Human vs Human", QVariant(0));
    m_modeSelector->addItem("Human vs AI",    QVariant(1));
    m_modeSelector->setStyleSheet(
        "QComboBox { background:#5a4e3c; color:white; border-radius:5px; "
        "            padding:4px 8px; font-size:13px; min-width:130px; }"
        "QComboBox::drop-down { border:none; }"
        "QComboBox QAbstractItemView { background:#4a3e2c; color:white; }"
    );
    connect(m_modeSelector,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onModeChanged);
    layout->addWidget(m_modeSelector);

    // NEW: AI Difficulty selector
    m_difficultyLabel = new QLabel("AI:");
    m_difficultyLabel->setStyleSheet("color: #c0b090; font-size: 13px;");
    layout->addWidget(m_difficultyLabel);

    m_difficultySelector = new QComboBox();
    m_difficultySelector->addItem("Easy",   QVariant(static_cast<int>(Game::AIDifficulty::Easy)));
    m_difficultySelector->addItem("Medium", QVariant(static_cast<int>(Game::AIDifficulty::Medium)));
    m_difficultySelector->addItem("Hard",   QVariant(static_cast<int>(Game::AIDifficulty::Hard)));
    m_difficultySelector->setStyleSheet(
        "QComboBox { background:#5a4e3c; color:white; border-radius:5px; "
        "            padding:4px 8px; font-size:13px; min-width:100px; }"
        "QComboBox::drop-down { border:none; }"
        "QComboBox QAbstractItemView { background:#4a3e2c; color:white; }"
    );
    connect(m_difficultySelector,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDifficultyChanged);
    layout->addWidget(m_difficultySelector);

    // Hide difficulty in HvH mode
    bool isHvAI = (m_game->getMode() == Game::Mode::HumanVsAI);
    m_difficultyLabel->setVisible(isHvAI);
    m_difficultySelector->setVisible(isHvAI);

    // NEW: Undo / Redo buttons
    m_undoButton = new QPushButton("↩ Undo");
    m_undoButton->setToolTip("Undo last move (Ctrl+Z)");
    m_undoButton->setEnabled(false);
    m_undoButton->setStyleSheet(
        "QPushButton { background:#4a5a6a; color:white; border-radius:5px; "
        "              padding:5px 10px; font-size:12px; }"
        "QPushButton:disabled { background:#3a3a3a; color:#888; }"
        "QPushButton:hover:!disabled { background:#5a6a7a; }"
    );
    connect(m_undoButton, &QPushButton::clicked, this, &MainWindow::onUndoClicked);
    layout->addWidget(m_undoButton);

    m_redoButton = new QPushButton("↪ Redo");
    m_redoButton->setToolTip("Redo undone move (Ctrl+Y)");
    m_redoButton->setEnabled(false);
    m_redoButton->setStyleSheet(
        "QPushButton { background:#4a5a6a; color:white; border-radius:5px; "
        "              padding:5px 10px; font-size:12px; }"
        "QPushButton:disabled { background:#3a3a3a; color:#888; }"
        "QPushButton:hover:!disabled { background:#5a6a7a; }"
    );
    connect(m_redoButton, &QPushButton::clicked, this, &MainWindow::onRedoClicked);
    layout->addWidget(m_redoButton);

    // Reset button
    m_resetButton = new QPushButton("⟳  New Game");
    m_resetButton->setStyleSheet(
        "QPushButton { background:#7a3030; color:white; border-radius:5px; "
        "              padding:5px 14px; font-size:13px; font-weight:bold; }"
        "QPushButton:hover { background:#9a4040; }"
        "QPushButton:pressed { background:#5a2020; }"
    );
    connect(m_resetButton, &QPushButton::clicked,
            this,           &MainWindow::onResetClicked);
    layout->addWidget(m_resetButton);

    return bar;
}

// ============================================================
//  createStatusPanel
// ============================================================
QWidget* MainWindow::createStatusPanel()
{
    QWidget* panel = new QWidget();
    panel->setStyleSheet("background-color: #3a2e1c; border-radius: 8px;");

    QVBoxLayout* vl = new QVBoxLayout(panel);
    vl->setContentsMargins(12, 10, 12, 10);
    vl->setSpacing(8);

    QHBoxLayout* row1 = new QHBoxLayout();

    m_statusLabel = new QLabel("Player 1's turn");
    m_statusLabel->setStyleSheet("color: #f0e8d0; font-size: 15px; font-weight: bold;");
    row1->addWidget(m_statusLabel);

    row1->addStretch(1);

    m_wallsLabel = new QLabel("Walls  ●  P1: 10   P2: 10");
    m_wallsLabel->setStyleSheet("color: #c0b090; font-size: 13px;");
    row1->addWidget(m_wallsLabel);

    vl->addLayout(row1);

    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #5a4e3c;");
    vl->addWidget(sep);

    QHBoxLayout* row2 = new QHBoxLayout();

    m_modeLabel = new QLabel("Input mode:");
    m_modeLabel->setStyleSheet("color: #a09070; font-size: 12px;");
    row2->addWidget(m_modeLabel);

    m_movePawnBtn = new QPushButton("♟  Move Pawn");
    m_movePawnBtn->setCheckable(true);
    m_movePawnBtn->setChecked(true);
    m_movePawnBtn->setToolTip("Click a highlighted cell to move your pawn");
    applyModeButtonStyle(m_movePawnBtn);
    connect(m_movePawnBtn, &QPushButton::clicked,
            this,          &MainWindow::onMovePawnModeClicked);
    row2->addWidget(m_movePawnBtn);

    m_placeWallBtn = new QPushButton("━  Place Wall  (right-click to rotate)");
    m_placeWallBtn->setCheckable(true);
    m_placeWallBtn->setChecked(false);
    m_placeWallBtn->setToolTip("Click between cells to place a wall. Right-click to rotate.");
    applyModeButtonStyle(m_placeWallBtn);
    connect(m_placeWallBtn, &QPushButton::clicked,
            this,           &MainWindow::onPlaceWallModeClicked);
    row2->addWidget(m_placeWallBtn);

    row2->addStretch(1);
    vl->addLayout(row2);

    return panel;
}

void MainWindow::applyModeButtonStyle(QPushButton* btn)
{
    btn->setStyleSheet(
        "QPushButton { background:#4a3e2c; color:#c0b090; border-radius:5px; "
        "              padding:6px 14px; font-size:13px; }"
        "QPushButton:checked  { background:#7a6030; color:white; font-weight:bold; }"
        "QPushButton:hover    { background:#5a4e3c; }"
        "QPushButton:pressed  { background:#3a2e1c; }"
    );
}

// ============================================================
//  onPawnMoveRequested
// ============================================================
void MainWindow::onPawnMoveRequested(const Position& target)
{
    bool accepted = m_game->moveCurrentPlayer(target);

    m_boardWidget->refreshBoard();
    updateStatusLabels();

    if (!accepted)
    {
        flashInvalidAction();
    }
    else if (m_game->isGameOver())
    {
        showWinnerMessage();
    }
}

// ============================================================
//  onWallPlaceRequested
// ============================================================
void MainWindow::onWallPlaceRequested(const Wall& wall)
{
    bool accepted = m_game->placeWallForCurrentPlayer(wall);

    m_boardWidget->refreshBoard();
    updateStatusLabels();

    if (!accepted)
    {
        flashInvalidAction();
    }
    else if (m_game->isGameOver())
    {
        showWinnerMessage();
    }
}

// ============================================================
//  onResetClicked
// ============================================================
void MainWindow::onResetClicked()
{
    m_game->resetGame();
    m_game->setInputMode(Game::InputMode::MovePawn);
    m_movePawnBtn->setChecked(true);
    m_placeWallBtn->setChecked(false);

    m_boardWidget->refreshBoard();
    updateStatusLabels();

    m_flashTimer->stop();
    m_statusLabel->setStyleSheet("color: #f0e8d0; font-size: 15px; font-weight: bold;");
}

// ============================================================
//  onModeChanged
// ============================================================
void MainWindow::onModeChanged(int index)
{
    Game::Mode newMode = (index == 0) ? Game::Mode::HumanVsHuman
                                      : Game::Mode::HumanVsAI;

    // Preserve current difficulty when switching modes
    Game::AIDifficulty currentDiff = m_game->getAIDifficulty();
    m_game = std::make_unique<Game>(newMode, currentDiff);
    m_boardWidget->setGame(m_game.get());

    m_game->setInputMode(Game::InputMode::MovePawn);
    m_movePawnBtn->setChecked(true);
    m_placeWallBtn->setChecked(false);

    // Show/hide difficulty selector
    bool isHvAI = (newMode == Game::Mode::HumanVsAI);
    m_difficultyLabel->setVisible(isHvAI);
    m_difficultySelector->setVisible(isHvAI);

    m_boardWidget->refreshBoard();
    updateStatusLabels();

    m_flashTimer->stop();
    m_statusLabel->setStyleSheet("color: #f0e8d0; font-size: 15px; font-weight: bold;");
}

// ============================================================
//  onMovePawnModeClicked / onPlaceWallModeClicked
// ============================================================
void MainWindow::onMovePawnModeClicked()
{
    m_game->setInputMode(Game::InputMode::MovePawn);
    m_movePawnBtn->setChecked(true);
    m_placeWallBtn->setChecked(false);
    m_boardWidget->refreshBoard();
    updateStatusLabels();
}

void MainWindow::onPlaceWallModeClicked()
{
    // Guard: already in wall mode, nothing to do
    if (m_game->getInputMode() == Game::InputMode::PlaceWall)
        return;

    m_game->setInputMode(Game::InputMode::PlaceWall);
    m_placeWallBtn->setChecked(true);
    m_movePawnBtn->setChecked(false);
    m_boardWidget->refreshBoard();
    updateStatusLabels();
}

// ============================================================
//  onToggleValidMoves
// ============================================================
void MainWindow::onToggleValidMoves(bool checked)
{
    m_boardWidget->setShowValidMoves(checked);
}

// ============================================================
//  NEW: onUndoClicked
// ============================================================
void MainWindow::onUndoClicked()
{
    if (m_game->undo())
    {
        m_boardWidget->refreshBoard();
        updateStatusLabels();
    }
}

// ============================================================
//  NEW: onRedoClicked
// ============================================================
void MainWindow::onRedoClicked()
{
    if (m_game->redo())
    {
        m_boardWidget->refreshBoard();
        updateStatusLabels();
    }
}

// ============================================================
//  NEW: onDifficultyChanged
// ============================================================
void MainWindow::onDifficultyChanged(int index)
{
    Game::AIDifficulty newDiff = static_cast<Game::AIDifficulty>(
        m_difficultySelector->itemData(index).toInt());
    m_game->setAIDifficulty(newDiff);
    updateStatusLabels();

    // Restore focus so keyboard shortcuts work
    m_boardWidget->setFocus();
}

// ============================================================
//  updateStatusLabels
// ============================================================
void MainWindow::updateStatusLabels()
{
    QString statusText = QString::fromStdString(m_game->getStatusMessage());
    m_statusLabel->setText(statusText);

    int w0 = m_game->getPlayer(0).getWallCount();
    int w1 = m_game->getPlayer(1).getWallCount();

    auto wallPips = [](int count) -> QString {
        QString pips;
        for (int i = 0; i < count;       ++i) pips += "▪";
        for (int i = count; i < Player::STARTING_WALLS; ++i) pips += "·";
        return pips;
    };

    m_wallsLabel->setText(
        QString("P1 [%1] %2   P2 [%3] %4")
            .arg(w0).arg(wallPips(w0))
            .arg(w1).arg(wallPips(w1))
    );

    QString modeStr = (m_game->getInputMode() == Game::InputMode::MovePawn)
                      ? "Move Pawn"
                      : "Place Wall  (right-click to rotate)";
    m_modeLabel->setText("Mode: " + modeStr);

    const Player& current = m_game->getCurrentPlayer();
    m_placeWallBtn->setEnabled(current.hasWallsRemaining() && !m_game->isGameOver());
    m_movePawnBtn->setEnabled(!m_game->isGameOver());

    // NEW: Update undo/redo button states
    m_undoButton->setEnabled(m_game->canUndo());
    m_redoButton->setEnabled(m_game->canRedo());
}

// ============================================================
//  showWinnerMessage
// ============================================================
void MainWindow::showWinnerMessage()
{
    int winnerIdx = m_game->getWinnerIndex();
    if (winnerIdx < 0) return;

    const Player& winner = m_game->getPlayer(winnerIdx);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Game Over");
    msgBox.setText(
        QString("<h2>%1 wins! 🏆</h2>")
            .arg(QString::fromStdString(winner.getName()))
    );
    msgBox.setInformativeText("Would you like to play again?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #3a2e1c; color: white; }"
        "QLabel { color: #f0e8d0; }"
        "QPushButton { background:#7a6030; color:white; padding:6px 20px; border-radius:4px; }"
    );

    int result = msgBox.exec();
    if (result == QMessageBox::Yes)
    {
        onResetClicked();
    }
}

// ============================================================
//  flashInvalidAction
// ============================================================
void MainWindow::flashInvalidAction()
{
    m_statusLabel->setStyleSheet(
        "color: #ff6060; font-size: 15px; font-weight: bold;"
    );
    m_statusLabel->setText("✗  Invalid move!");
    m_flashTimer->start(600);
}

// ============================================================
//  keyPressEvent — handle Ctrl+Z / Ctrl+Y globally
// ============================================================
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        switch (event->key())
        {
            case Qt::Key_Z:
                onUndoClicked();
                return;
            case Qt::Key_Y:
                onRedoClicked();
                return;
        }
    }

    switch (event->key())
    {
        case Qt::Key_W:
            // Toggle mode based on current button state (more reliable than Game state)
            if (m_movePawnBtn->isChecked())
                onPlaceWallModeClicked();
            else
                onMovePawnModeClicked();
            return;
        case Qt::Key_R:
            m_boardWidget->rotateWallOrientation();
            return;
        case Qt::Key_F2:
            onResetClicked();
            return;
    }

    QMainWindow::keyPressEvent(event);
}
