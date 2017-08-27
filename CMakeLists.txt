cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 14)

project (hunt_the_wumpus)

add_executable (hunt_the_wumpus src/main.cpp src/game.cpp src/console_game.cpp)
