cmake_minimum_required(VERSION 3.5)

project(GameMatcher LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick Sql Widgets REQUIRED)


add_executable(GameMatcher
        src/main.cpp
        src/MainWindow.ui
        res/qml/qml.qrc
        res/db/queries.qrc
        src/clubrepository.cpp
        src/clubrepository.h
        src/models.h
        src/gamematcher.cpp
        src/gamematcher.h
        src/gamestats.h src/dbutils.h src/dbutils.cpp src/MainWindow.h src/MainWindow.cpp src/NewClubDialog.h src/NewClubDialog.cpp src/WelcomePage.cpp src/WelcomePage.h src/ClubPage.cpp src/ClubPage.h src/SessionDialog.cpp src/SessionDialog.h src/NewSessionDialog.cpp src/NewSessionDialog.h src/EmptySessionPage.cpp src/EmptySessionPage.h src/CourtDisplay.cpp src/CourtDisplay.h)

target_compile_definitions(GameMatcher
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
        -DQT_NO_CAST_FROM_ASCII=1)
target_link_libraries(GameMatcher
        PRIVATE Qt5::Core Qt5::Quick Qt5::Sql Qt5::Widgets)
