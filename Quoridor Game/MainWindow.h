#pragma once

// ============================================================
//  MainWindow.h
//
//  The top-level application window for the Quoridor GUI.
//
//  NEW (v2.0):
//    • AI Difficulty selector (Easy / Medium / Hard)
//    • Undo / Redo buttons with keyboard shortcuts
//    • Right-click wall rotation support
// ============================================================

#include <QMainWindow>
#include <memory>

#include "Game.h"

class BoardWidget;
class QLabel;
class QPushButton;
class QComboBox;
class QButtonGroup;
class QToolButton;
class QTimer;
class QKeyEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Game::Mode mode = Game::Mode::HumanVsHuman,
                      Game::AIDifficulty diff = Game::AIDifficulty::Medium,
                      QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:

    // BoardWidget signals
    void onPawnMoveRequested(const Position& target);
    void onWallPlaceRequested(const Wall& wall);

    // UI controls
    void onResetClicked();
    void onModeChanged(int index);
    void onMovePawnModeClicked();
    void onPlaceWallModeClicked();
    void onToggleValidMoves(bool checked);

    // NEW: Undo / Redo
    void onUndoClicked();
    void onRedoClicked();

    // NEW: AI Difficulty changed
    void onDifficultyChanged(int index);

private:

    void setupUI();
    QWidget* createToolbar();
    QWidget* createStatusPanel();
    void applyModeButtonStyle(QPushButton* btn);

    void updateStatusLabels();
    void showWinnerMessage();
    void flashInvalidAction();

    std::unique_ptr<Game> m_game;
    BoardWidget* m_boardWidget;

    QLabel* m_statusLabel;
    QLabel* m_wallsLabel;
    QLabel* m_modeLabel;

    QComboBox*    m_modeSelector;
    QPushButton*  m_resetButton;

    // NEW: AI Difficulty selector
    QComboBox*    m_difficultySelector;
    QLabel*       m_difficultyLabel;

    QPushButton*  m_movePawnBtn;
    QPushButton*  m_placeWallBtn;
    QPushButton*  m_highlightBtn;

    // NEW: Undo / Redo buttons
    QPushButton*  m_undoButton;
    QPushButton*  m_redoButton;

    QTimer* m_flashTimer;
};
