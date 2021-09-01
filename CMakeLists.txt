cmake_minimum_required(VERSION 3.11)
project(Tiles LANGUAGES CXX)

add_executable(tiles
    src/main.cpp
)

find_package(ftxui REQUIRED)
target_link_libraries(tiles
    PRIVATE ftxui::component
)
