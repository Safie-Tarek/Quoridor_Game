#pragma once

// ============================================================
//  GameWindow.h
//
//  The top-level application window.  Owns the Game object
//  and the BoardWidget.  Provides all UI chrome around the
//  board: status label, wall-count display, buttons.
//
//  NEW (v2.0):
//    • AI Difficulty selector (Easy / Medium / Hard)
//    • Undo / Redo buttons with keyboard shortcuts (Ctrl+Z / Ctrl+Y)
//    • Visual feedback for undo/redo availability
// ============================================================

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include "Game.h"

class BoardWidget;
class QComboBox;

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(Game::Mode mode = Game::Mode::HumanVsAI,
                        QWidget* parent = nullptr);

protected:
    // Keyboard shortcuts:
    //   W      = toggle mode
    //   R      = rotate wall
    //   F2     = new game
    //   Ctrl+Z = undo
    //   Ctrl+Y = redo
    void keyPressEvent(QKeyEvent* event) override;

    void onModeChanged(int index);

private slots:
    void onGameStateChanged();
    void onResetClicked();
    void onToggleModeClicked();
    void onUndoClicked();
    void onRedoClicked();
    void onDifficultyChanged(int index);

private:

    // ---- Owned objects --------------------------------------
    Game*        m_game;
    BoardWidget* m_boardWidget;

    // ---- UI elements ----------------------------------------
    QLabel*      m_statusLabel;
    QLabel*      m_p0WallsLabel;
    QLabel*      m_p1WallsLabel;
    QLabel*      m_modeLabel;
    QPushButton* m_resetButton;
    QPushButton* m_toggleModeButton;

    // NEW: Undo / Redo buttons
    QPushButton* m_undoButton;
    QPushButton* m_redoButton;

    // NEW: AI Difficulty selector
    QComboBox*   m_difficultySelector;
    QLabel*      m_difficultyLabel;

    QComboBox*   m_modeSelector;

    // ---- Setup helpers --------------------------------------
    void setupUI();
    void refreshUI();
};
