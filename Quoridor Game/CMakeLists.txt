cmake_minimum_required(VERSION 3.19)
project(Quoridor VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Widgets QUIET)

if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Found Qt6")
else()
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Found Qt5")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

set(ALL_SOURCES
    main.cpp
    Position.h
    Wall.h
    Wall.cpp
    Board.h
    Board.cpp
    Player.h
    Player.cpp
    Game.h
    Game.cpp
    GameState.h
    AIPlayer.h
    AIPlayer.cpp
    BoardWidget.h
    BoardWidget.cpp
    GameWindow.h
    GameWindow.cpp
    MainWindow.h
    MainWindow.cpp
)

add_executable(Quoridor ${ALL_SOURCES})

target_include_directories(Quoridor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(Quoridor PRIVATE Qt6::Core Qt6::Widgets)
else()
    target_link_libraries(Quoridor PRIVATE Qt5::Core Qt5::Widgets)
endif()

if(WIN32)
    set_target_properties(Quoridor PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
endif()

if(MSVC)
    target_compile_options(Quoridor PRIVATE /W3)
else()
    target_compile_options(Quoridor PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32 AND QT_VERSION_MAJOR EQUAL 6)
    find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT)
        add_custom_command(TARGET Quoridor POST_BUILD
            COMMAND "${WINDEPLOYQT}" "--no-translations" "$<TARGET_FILE:Quoridor>"
            COMMENT "Running windeployqt..."
        )
    endif()
endif()
