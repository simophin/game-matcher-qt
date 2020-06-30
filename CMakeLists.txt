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
        res/qml/qml.qrc
        res/db/queries.qrc
        src/ClubRepository.cpp
        src/ClubRepository.h
        src/models.h
        src/GameMatcher.cpp
        src/GameMatcher.h
        src/GameStats.h
        src/DbUtils.h
        src/DbUtils.cpp
        src/MainWindow.h
        src/MainWindow.cpp
        src/MainWindow.ui
        src/NewClubDialog.h
        src/NewClubDialog.cpp
        src/NewClubDialog.ui
        src/WelcomePage.cpp
        src/WelcomePage.h
        src/WelcomePage.ui
        src/ClubPage.cpp
        src/ClubPage.h
        src/ClubPage.ui
        src/SessionDialog.cpp
        src/SessionDialog.h
        src/SessionDialog.ui
        src/NewSessionDialog.cpp
        src/NewSessionDialog.h
        src/NewSessionDialog.ui
        src/EmptySessionPage.cpp
        src/EmptySessionPage.h
        src/EmptySessionPage.ui
        src/CourtDisplay.cpp
        src/CourtDisplay.h
        src/CourtDisplay.ui
        src/Adapter.h
        src/EditMemberDialog.cpp
        src/EditMemberDialog.h
        src/EditMemberDialog.ui
        src/MemberSelectDialog.ui
        src/MemberSelectDialog.cpp
        src/MemberSelectDialog.h
        src/FakeNames.cpp
        src/FakeNames.h
        src/MemberFilter.h
        src/CheckInDialog.cpp
        src/CheckInDialog.h
        src/CheckInDialog.ui
        )

target_compile_definitions(GameMatcher
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
        -DQT_NO_CAST_FROM_ASCII=1)
target_link_libraries(GameMatcher
        PRIVATE Qt5::Core Qt5::Quick Qt5::Sql Qt5::Widgets)
