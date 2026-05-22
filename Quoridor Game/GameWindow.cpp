// ============================================================
//  GameWindow.cpp
//
//  NEW (v2.0):
//    • AI Difficulty selector dropdown
//    • Undo / Redo buttons with Ctrl+Z / Ctrl+Y shortcuts
//    • Dynamic enable/disable of undo/redo based on history
// ============================================================

#include "GameWindow.h"
#include "BoardWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QFrame>
#include <QFont>
#include <QComboBox>
#include <QShortcut>
#include <QScreen>
#include <QGuiApplication>

// ============================================================
//  Constructor
// ============================================================
GameWindow::GameWindow(Game::Mode mode, QWidget* parent)
    : QMainWindow(parent)
    , m_game(nullptr)
    , m_boardWidget(nullptr)
    , m_statusLabel(nullptr)
    , m_p0WallsLabel(nullptr)
    , m_p1WallsLabel(nullptr)
    , m_modeLabel(nullptr)
    , m_resetButton(nullptr)
    , m_toggleModeButton(nullptr)
    , m_undoButton(nullptr)
    , m_redoButton(nullptr)
    , m_difficultySelector(nullptr)
    , m_difficultyLabel(nullptr)
    , m_modeSelector(nullptr)
{
    setWindowTitle("Quoridor");

    m_game = new Game(mode);

    setupUI();
    refreshUI();
}

// ============================================================
//  setupUI()
// ============================================================
void GameWindow::setupUI()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* rootLayout = new QHBoxLayout(central);
    rootLayout->setSpacing(16);
    rootLayout->setContentsMargins(16, 16, 16, 16);

    // ---- Board widget ---------------------------------------
    m_boardWidget = new BoardWidget(m_game, this);
    rootLayout->addWidget(m_boardWidget);

    // ---- Right-side panel -----------------------------------
    QVBoxLayout* panelLayout = new QVBoxLayout();
    panelLayout->setSpacing(12);

    // Title
    QLabel* titleLabel = new QLabel("QUORIDOR", this);
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(titleLabel);

    // Divider
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    panelLayout->addWidget(line);

    // ---- Mode selector --------------------------------------
    QHBoxLayout* modeLayout = new QHBoxLayout();
    QLabel* modeLabel = new QLabel("Mode:", this);
    modeLayout->addWidget(modeLabel);

    m_modeSelector = new QComboBox(this);
    m_modeSelector->addItem("Human vs AI",    QVariant(static_cast<int>(Game::Mode::HumanVsAI)));
    m_modeSelector->addItem("Human vs Human", QVariant(static_cast<int>(Game::Mode::HumanVsHuman)));

    int currentModeIndex = (m_game->getMode() == Game::Mode::HumanVsAI) ? 0 : 1;
    m_modeSelector->setCurrentIndex(currentModeIndex);

    modeLayout->addWidget(m_modeSelector);
    panelLayout->addLayout(modeLayout);

    // ---- AI Difficulty selector (NEW) ----------------------
    QHBoxLayout* diffLayout = new QHBoxLayout();
    m_difficultyLabel = new QLabel("AI Difficulty:", this);
    diffLayout->addWidget(m_difficultyLabel);

    m_difficultySelector = new QComboBox(this);
    m_difficultySelector->addItem("Easy",   QVariant(static_cast<int>(Game::AIDifficulty::Easy)));
    m_difficultySelector->addItem("Medium", QVariant(static_cast<int>(Game::AIDifficulty::Medium)));
    m_difficultySelector->addItem("Hard",   QVariant(static_cast<int>(Game::AIDifficulty::Hard)));

    int currentDiffIndex = static_cast<int>(m_game->getAIDifficulty());
    m_difficultySelector->setCurrentIndex(currentDiffIndex);

    diffLayout->addWidget(m_difficultySelector);
    panelLayout->addLayout(diffLayout);

    // Hide difficulty selector in HvH mode
    bool isHvAI = (m_game->getMode() == Game::Mode::HumanVsAI);
    m_difficultyLabel->setVisible(isHvAI);
    m_difficultySelector->setVisible(isHvAI);

    // Divider
    QFrame* line2 = new QFrame(this);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    panelLayout->addWidget(line2);

    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    QFont statusFont;
    statusFont.setPointSize(11);
    m_statusLabel->setFont(statusFont);
    panelLayout->addWidget(m_statusLabel);

    // Wall counts
    m_p0WallsLabel = new QLabel(this);
    m_p1WallsLabel = new QLabel(this);
    panelLayout->addWidget(m_p0WallsLabel);
    panelLayout->addWidget(m_p1WallsLabel);

    // Current mode display
    m_modeLabel = new QLabel(this);
    m_modeLabel->setAlignment(Qt::AlignCenter);
    QFont modeFont;
    modeFont.setItalic(true);
    m_modeLabel->setFont(modeFont);
    panelLayout->addWidget(m_modeLabel);

    panelLayout->addStretch();

    // ---- Undo / Redo buttons (NEW) ---------------------------
    QHBoxLayout* undoRedoLayout = new QHBoxLayout();

    m_undoButton = new QPushButton("↩ Undo  [Ctrl+Z]", this);
    m_undoButton->setToolTip("Undo the last move");
    m_undoButton->setEnabled(false);
    undoRedoLayout->addWidget(m_undoButton);

    m_redoButton = new QPushButton("↪ Redo  [Ctrl+Y]", this);
    m_redoButton->setToolTip("Redo the undone move");
    m_redoButton->setEnabled(false);
    undoRedoLayout->addWidget(m_redoButton);

    panelLayout->addLayout(undoRedoLayout);

    // Toggle mode button
    m_toggleModeButton = new QPushButton("Toggle Mode  [W]", this);
    panelLayout->addWidget(m_toggleModeButton);

    // Reset button
    m_resetButton = new QPushButton("New Game  [F2]", this);
    panelLayout->addWidget(m_resetButton);

    // Keyboard hint
    QString hintText = QString("Controls:\n"
                               "  W - toggle move / wall mode\n"
                               "  R - rotate wall orientation\n"
                               "  F2 - new game\n"
                               "  Ctrl+Z - undo\n"
                               "  Ctrl+Y - redo\n"
                               "  Left-click - perform action\n"
                               "  Right-click - rotate wall");
    QLabel* hintLabel = new QLabel(hintText, this);
    hintLabel->setStyleSheet("color: grey; font-size: 9pt;");
    panelLayout->addWidget(hintLabel);

    rootLayout->addLayout(panelLayout);

    // ---- Signal / slot connections --------------------------
    connect(m_boardWidget, &BoardWidget::gameStateChanged,
            this,          &GameWindow::onGameStateChanged);

    connect(m_resetButton,      &QPushButton::clicked,
            this,               &GameWindow::onResetClicked);
    connect(m_toggleModeButton,   &QPushButton::clicked,
            this,               &GameWindow::onToggleModeClicked);
    connect(m_undoButton,         &QPushButton::clicked,
            this,               &GameWindow::onUndoClicked);
    connect(m_redoButton,         &QPushButton::clicked,
            this,               &GameWindow::onRedoClicked);

    connect(m_modeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,           &GameWindow::onModeChanged);
    connect(m_difficultySelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,               &GameWindow::onDifficultyChanged);

    // ---- Global keyboard shortcuts (QShortcut) ------------
    QShortcut* shortcutW = new QShortcut(QKeySequence("W"), this);
    connect(shortcutW, &QShortcut::activated, this, &GameWindow::onToggleModeClicked);

    QShortcut* shortcutR = new QShortcut(QKeySequence("R"), this);
    connect(shortcutR, &QShortcut::activated, this, [this]() {
        m_boardWidget->rotateWallOrientation();
    });

    QShortcut* shortcutF2 = new QShortcut(QKeySequence("F2"), this);
    connect(shortcutF2, &QShortcut::activated, this, &GameWindow::onResetClicked);

    QShortcut* shortcutUndo = new QShortcut(QKeySequence("Ctrl+Z"), this);
    connect(shortcutUndo, &QShortcut::activated, this, &GameWindow::onUndoClicked);

    QShortcut* shortcutRedo = new QShortcut(QKeySequence("Ctrl+Y"), this);
    connect(shortcutRedo, &QShortcut::activated, this, &GameWindow::onRedoClicked);

    // Size window to fill screen height
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect available = screen->availableGeometry();
    int targetHeight = available.height() - 40;
    int targetWidth = m_boardWidget->width() + 320;
    resize(targetWidth, targetHeight);
    move(available.center().x() - targetWidth/2, 20);
}

// ============================================================
//  refreshUI()
// ============================================================
void GameWindow::refreshUI()
{
    m_statusLabel->setText(
        QString::fromStdString(m_game->getStatusMessage()));

    m_p0WallsLabel->setText(
        QString("[P1] %1  walls: %2")
        .arg(QString::fromStdString(m_game->getPlayer(0).getName()))
        .arg(m_game->getPlayer(0).getWallCount()));

    m_p1WallsLabel->setText(
        QString("[P2] %1  walls: %2")
        .arg(QString::fromStdString(m_game->getPlayer(1).getName()))
        .arg(m_game->getPlayer(1).getWallCount()));

    bool wallMode = (m_game->getInputMode() == Game::InputMode::PlaceWall);
    m_modeLabel->setText(wallMode ? "Mode: Place Wall" : "Mode: Move Pawn");
    m_modeLabel->setStyleSheet(
        wallMode ? "color: #8B0000; font-weight: bold;"
                 : "color: #00008B; font-weight: bold;");

    // NEW: Update undo/redo button states
    m_undoButton->setEnabled(m_game->canUndo());
    m_redoButton->setEnabled(m_game->canRedo());

    // Update difficulty selector visibility
    bool isHvAI = (m_game->getMode() == Game::Mode::HumanVsAI);
    m_difficultyLabel->setVisible(isHvAI);
    m_difficultySelector->setVisible(isHvAI);
}

// ============================================================
//  Event handlers
// ============================================================
void GameWindow::onGameStateChanged()
{
    refreshUI();
}

void GameWindow::onResetClicked()
{
    m_game->resetGame();
    m_boardWidget->update();
    refreshUI();
}

void GameWindow::onToggleModeClicked()
{
    m_boardWidget->toggleInputMode();
    refreshUI();
}

// NEW: Undo handler
void GameWindow::onUndoClicked()
{
    if (m_game->undo())
    {
        m_boardWidget->update();
        refreshUI();
    }
}

// NEW: Redo handler
void GameWindow::onRedoClicked()
{
    if (m_game->redo())
    {
        m_boardWidget->update();
        refreshUI();
    }
}

// NEW: Difficulty changed handler
void GameWindow::onDifficultyChanged(int index)
{
    Game::AIDifficulty newDiff = static_cast<Game::AIDifficulty>(
        m_difficultySelector->itemData(index).toInt());
    m_game->setAIDifficulty(newDiff);
    refreshUI();
}

void GameWindow::onModeChanged(int index)
{
    Game::Mode newMode = static_cast<Game::Mode>(
        m_modeSelector->itemData(index).toInt());

    if (m_game->getMode() == newMode)
        return;

    delete m_game;
    m_game = new Game(newMode);

    m_boardWidget->setGame(m_game);

    refreshUI();
}

void GameWindow::keyPressEvent(QKeyEvent* event)
{
    QMainWindow::keyPressEvent(event);
}
