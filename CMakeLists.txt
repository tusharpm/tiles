cmake_minimum_required(VERSION 3.11)
project(tiles LANGUAGES CXX)

add_executable(fifteen
    src/fifteen.cpp
)

find_package(ftxui REQUIRED)
target_link_libraries(fifteen
    PRIVATE ftxui::component
)

add_executable(2048
    src/2048.cpp
)
target_link_libraries(2048
    PRIVATE ftxui::component
)
