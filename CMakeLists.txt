cmake_minimum_required(VERSION 3.5)

add_subdirectory(third_party/qtsqlx)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(GameMatcher LANGUAGES CXX)

option(BUILD_TESTS "Skip buildling tests" ON)

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
        src/ClubRepositoryInternal.h)

qt5_wrap_cpp(SOURCES ${HEADERS})

list(APPEND HEADERS
        src/Adapter.h
        src/GameMatcher.h
        src/GameStats.h
        src/FakeNames.h
        src/MemberFilter.h
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

add_library(GameMatcher_archive
        OBJECT
        ${SOURCES}
        ${HEADERS}
        src/ClubRepository.cpp
        src/GameMatcher.cpp
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
        src/models.cpp
        )

target_compile_definitions(GameMatcher_archive PRIVATE -DQT_NO_CAST_FROM_ASCII=1 -DAPP_VERSION_MAJOR=1 -DAPP_VERSION_MINOR=0)
target_include_directories(GameMatcher_archive PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})
target_link_libraries(GameMatcher_archive PUBLIC Qt5::Core Qt5::Sql Qt5::Widgets Qt5::Multimedia Qt5::Svg QtSQLx)

if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_compile_options(GameMatcher_archive PUBLIC -mwindows -static)
    target_link_options(GameMatcher_archive PUBLIC -static)
endif ()

add_executable(GameMatcher WIN32 src/main.cpp)
target_link_libraries(GameMatcher GameMatcher_archive)

qt5_import_plugins(GameMatcher INCLUDE Qt5::QSvgPlugin)


if (BUILD_TESTS)
    find_package(Catch2 REQUIRED)
    find_package(Qt5 COMPONENTS Test REQUIRED)

    add_executable(GameMatcher_test
            src/test/TestUtils.h
            src/test/NewClubRepositoryTest.cpp
            src/test/main.cpp
            )
    target_link_libraries(GameMatcher_test GameMatcher_archive Catch2::Catch2 Qt5::Test)
    target_compile_definitions(GameMatcher_test PRIVATE CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS)
endif ()
