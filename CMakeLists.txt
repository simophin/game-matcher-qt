cmake_minimum_required(VERSION 3.5)

add_subdirectory(third_party/spdlog)

project(GameMatcher LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Sql Widgets REQUIRED)


add_executable(GameMatcher
        src/main.cpp
        res/db/queries.qrc
        res/images/images.qrc
        res/icons/icons.qrc
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
        src/SessionWindow.cpp
        src/SessionWindow.h
        src/SessionWindow.ui
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
        src/NewGameDialog.cpp
        src/NewGameDialog.ui
        src/NewGameDialog.h
        src/TypeUtils.h
        src/ClubRepositoryInternal.h
        src/FlowLayout.h
        src/FlowLayout.cpp
        src/span.h
        src/PlayerTablePage.cpp
        src/PlayerTablePage.h
        src/PlayerTablePage.ui
        src/CollectionUtils.h
        src/NameFormatUtils.h
        src/ToastEvent.cpp
        src/ToastEvent.h
        src/ToastDialog.cpp
        src/ToastDialog.h
        src/CombinationsFinder.h
        src/FastIntVector.h src/MatchingScore.h src/PlayerInfo.h src/MatchingScore.cpp src/CourtCombinationFinder.h src/CourtCombinationFinder.cpp src/SortingCourtCombinationFinder.cpp src/SortingCourtCombinationFinder.h)

target_compile_definitions(GameMatcher
        PRIVATE -DQT_NO_CAST_FROM_ASCII=1)
target_include_directories(GameMatcher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(GameMatcher
        PRIVATE Qt5::Core Qt5::Sql Qt5::Widgets spdlog)
