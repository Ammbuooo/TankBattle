cmake_minimum_required(VERSION 3.14)
project(TankBattle VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# FTXUI — FetchContent automatically downloads and builds FTXUI
include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG        v5.0.0
)
FetchContent_MakeAvailable(ftxui)

# Sources
set(SOURCES
    src/main.cpp
    src/game.cpp
    src/tank.cpp
    src/bullet.cpp
    src/map.cpp
)

add_executable(tank_battle ${SOURCES})

target_include_directories(tank_battle PRIVATE include)

target_link_libraries(tank_battle PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)

# Platform-specific settings
if(WIN32)
    # Windows: link against ws2_32 for networking (required by FTXUI)
    target_link_libraries(tank_battle PRIVATE ws2_32)
endif()
