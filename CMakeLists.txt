cmake_minimum_required(VERSION 3.16)
project(computer_graphics_lab3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(computer_graphics_lab3
        main.cpp
        mainwindow.cpp
        mainwindow.h
        canvas.cpp
        canvas.h
)
target_link_libraries(computer_graphics_lab3
        Qt::Core
        Qt::Gui
        Qt::Widgets
)
