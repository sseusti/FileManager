cmake_minimum_required(VERSION 4.1)
project(untitled1)

set(CMAKE_CXX_STANDARD 20)

add_executable(untitled1 main.cpp
        src/CommandHandler.cpp
        src/CommandHandler.h
        src/CommandParser.h
        src/CommandParser.cpp
)

find_package(termcolor REQUIRED)
target_link_libraries(untitled1 termcolor::termcolor)