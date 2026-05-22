QT += core widgets
RC_ICONS = appicon.ico
CONFIG += c++17
CONFIG -= console

TARGET = Quoridor
TEMPLATE = app

# C++17 standard
QMAKE_CXXFLAGS += -std=c++17

# Sources
SOURCES +=     main.cpp     Wall.cpp     Board.cpp     Player.cpp     Game.cpp     AIPlayer.cpp     BoardWidget.cpp     GameWindow.cpp     MainWindow.cpp

# Headers
HEADERS +=     Position.h     Wall.h     Board.h     Player.h     Game.h     GameState.h     AIPlayer.h     BoardWidget.h     GameWindow.h     MainWindow.h

# Platform-specific settings
win32 {
    # Windows-specific settings
    CONFIG += windows
}

macx {
    # macOS-specific settings
    CONFIG += app_bundle
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
