// ============================================================
//  main.cpp
//  Application entry point for the Quoridor Qt GUI.
//
//  NEW (v2.0):
//    • Startup dialog includes AI Difficulty selection
//    • Uses MainWindow as the primary window (feature-rich)
// ============================================================

#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialog>
#include "MainWindow.h"

// Startup dialog for selecting game mode and AI difficulty
class StartupDialog : public QDialog
{

public:
    StartupDialog(QWidget* parent = nullptr)
        : QDialog(parent)
        , m_selectedMode(Game::Mode::HumanVsAI)
        , m_selectedDifficulty(Game::AIDifficulty::Medium)
    {
        setWindowTitle("Quoridor");
        setFixedSize(400, 280);
        setStyleSheet(
            "QDialog { background-color: #2a2018; }"
            "QLabel { color: #f0e8d0; font-size: 14px; }"
            "QPushButton { background:#7a6030; color:white; padding:8px 20px; "
            "            border-radius:5px; font-size:13px; min-width:120px; }"
            "QPushButton:hover { background:#9a8040; }"
            "QPushButton:pressed { background:#5a4020; }"
            "QComboBox { background:#5a4e3c; color:white; border-radius:5px; "
            "          padding:4px 8px; font-size:13px; min-width:130px; }"
            "QComboBox::drop-down { border:none; }"
            "QComboBox QAbstractItemView { background:#4a3e2c; color:white; }"
        );

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(16);
        mainLayout->setContentsMargins(24, 24, 24, 24);

        // Title
        QLabel* title = new QLabel("<h2 style='color:#d4a84b;'>Welcome to Quoridor!</h2>");
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);

        // Mode selection
        QHBoxLayout* modeLayout = new QHBoxLayout();
        QLabel* modeLabel = new QLabel("Game Mode:");
        modeLayout->addWidget(modeLabel);

        m_modeCombo = new QComboBox();
        m_modeCombo->addItem("Human vs AI",    QVariant(static_cast<int>(Game::Mode::HumanVsAI)));
        m_modeCombo->addItem("Human vs Human", QVariant(static_cast<int>(Game::Mode::HumanVsHuman)));
        modeLayout->addWidget(m_modeCombo);
        mainLayout->addLayout(modeLayout);

        // Difficulty selection
        QHBoxLayout* diffLayout = new QHBoxLayout();
        m_diffLabel = new QLabel("AI Difficulty:");
        diffLayout->addWidget(m_diffLabel);

        m_diffCombo = new QComboBox();
        m_diffCombo->addItem("Easy",   QVariant(static_cast<int>(Game::AIDifficulty::Easy)));
        m_diffCombo->addItem("Medium", QVariant(static_cast<int>(Game::AIDifficulty::Medium)));
        m_diffCombo->addItem("Hard",   QVariant(static_cast<int>(Game::AIDifficulty::Hard)));
        m_diffCombo->setCurrentIndex(1); // Medium default
        diffLayout->addWidget(m_diffCombo);
        mainLayout->addLayout(diffLayout);

        // Connect mode change to show/hide difficulty
        connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int index) { onModeChanged(index); });

        mainLayout->addStretch();

        // Buttons
        QHBoxLayout* btnLayout = new QHBoxLayout();
        btnLayout->addStretch();

        QPushButton* startBtn = new QPushButton("Start Game");
        connect(startBtn, &QPushButton::clicked, this, &QDialog::accept);
        btnLayout->addWidget(startBtn);

        QPushButton* quitBtn = new QPushButton("Quit");
        connect(quitBtn, &QPushButton::clicked, this, &QDialog::reject);
        btnLayout->addWidget(quitBtn);

        mainLayout->addLayout(btnLayout);
    }

    Game::Mode getSelectedMode() const
    {
        return static_cast<Game::Mode>(m_modeCombo->currentData().toInt());
    }

    Game::AIDifficulty getSelectedDifficulty() const
    {
        return static_cast<Game::AIDifficulty>(m_diffCombo->currentData().toInt());
    }

private:
    void onModeChanged(int index)
    {
        bool isHvAI = (index == 0);
        m_diffLabel->setVisible(isHvAI);
        m_diffCombo->setVisible(isHvAI);
    }

private:
    QComboBox* m_modeCombo;
    QComboBox* m_diffCombo;
    QLabel*    m_diffLabel;
    Game::Mode m_selectedMode;
    Game::AIDifficulty m_selectedDifficulty;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Quoridor");
    app.setApplicationVersion("2.0");
    app.setStyle("Fusion");

    // Show startup dialog
    StartupDialog dialog;
    if (dialog.exec() != QDialog::Accepted)
        return 0;

    Game::Mode mode = dialog.getSelectedMode();
    Game::AIDifficulty diff = dialog.getSelectedDifficulty();
    // Create main window with selected settings
    MainWindow window(mode, diff);
    window.show();

    return app.exec();
}

