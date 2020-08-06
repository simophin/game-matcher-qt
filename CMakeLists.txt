cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.55.0 REQUIRED)

project(GameMatcher LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core Sql Widgets Multimedia Svg REQUIRED)

set(HEADERS
        src/ClubRepository.h
        src/models.h
        src/NewClubDialog.h
        src/WelcomePage.h
        src/ClubPage.h
        src/SessionPage.h
        src/NewSessionDialog.h
        src/EmptySessionPage.h
        src/CourtDisplay.h
        src/EditMemberDialog.h
        src/MemberSelectDialog.h
        src/CheckInDialog.h
        src/NewGameDialog.h
        src/PlayerTablePage.h
        src/ToastDialog.h
        src/MainWindow.h
        src/CourtDisplayLayout.h
        src/MemberLabel.h
        src/MemberImportDialog.h
        src/PlayerStatsDialog.h
        src/PlayerTableDialog.h
        src/ClubRepositoryModels.h
        src/ClubRepositoryInternal.h
        )

qt5_wrap_cpp(SOURCES ${HEADERS})

list(APPEND HEADERS
        src/Adapter.h
        src/GameMatcher.h
        src/GameStats.h
        src/FakeNames.h
        src/MemberFilter.h
        src/TypeUtils.h
        src/span.h
        src/CollectionUtils.h
        src/NameFormatUtils.h
        src/CombinationsFinder.h
        src/MatchingScore.h
        src/PlayerInfo.h
        src/CourtCombinationFinder.h
        src/BFCombinationFinder.h
        src/MemberPainter.h
        src/MemberMenu.h
        src/LastSelectedCourts.h
        )

qt5_add_resources(SOURCES
    res/db/queries.qrc
    res/images/images.qrc
    res/icons/icons.qrc
    res/fonts/fonts.qrc
    res/sound/sound.qrc)

qt5_wrap_ui(SOURCES
    src/NewClubDialog.ui
    src/WelcomePage.ui
    src/ClubPage.ui
    src/SessionPage.ui
    src/NewSessionDialog.ui
    src/EmptySessionPage.ui
    src/CourtDisplay.ui
    src/EditMemberDialog.ui
    src/MemberSelectDialog.ui
    src/CheckInDialog.ui
    src/NewGameDialog.ui
    src/PlayerTablePage.ui
    src/MainWindow.ui
    src/MemberImportDialog.ui
    src/PlayerStatsDialog.ui
    src/PlayerTableDialog.ui
    src/ToastDialog.ui
    )

add_executable(GameMatcher
        WIN32
        ${SOURCES}
        ${HEADERS}
        src/main.cpp
        src/ClubRepository.cpp
        src/GameMatcher.cpp
        src/DbUtils.cpp
        src/NewClubDialog.cpp
        src/WelcomePage.cpp
        src/ClubPage.cpp
        src/SessionPage.cpp
        src/NewSessionDialog.cpp
        src/EmptySessionPage.cpp
        src/CourtDisplay.cpp
        src/EditMemberDialog.cpp
        src/MemberSelectDialog.cpp
        src/FakeNames.cpp
        src/CheckInDialog.cpp
        src/NewGameDialog.cpp
        src/PlayerTablePage.cpp
        src/ToastDialog.cpp
        src/MatchingScore.cpp
        src/CourtCombinationFinder.cpp
        src/BFCombinationFinder.cpp
        src/MainWindow.cpp
        src/MemberPainter.cpp
        src/MemberMenu.cpp
        src/CourtDisplayLayout.cpp
        src/MemberLabel.cpp
        src/MemberImportDialog.cpp
        src/PlayerStatsDialog.cpp
        src/PlayerTableDialog.cpp
        )

target_compile_definitions(GameMatcher
        PRIVATE -DQT_NO_CAST_FROM_ASCII=1)
target_include_directories(GameMatcher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})
target_link_libraries(GameMatcher
        PRIVATE Qt5::Core Qt5::Sql Qt5::Widgets Qt5::Multimedia Qt::Svg)

qt5_import_plugins(GameMatcher
        INCLUDE_BY_TYPE imageformats Qt5::QSvgPlugin)
