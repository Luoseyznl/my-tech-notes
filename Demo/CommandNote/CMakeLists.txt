cmake_minimum_required(VERSION 3.18)
project(CommandNote)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(notepad
    src/Notepad.cpp
    src/ConsoleInterface.cpp
    src/main.cpp
)
