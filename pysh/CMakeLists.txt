cmake_minimum_required(VERSION 3.13)  # CMake version check
project(pysh)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 20)            # Enable c++20 standard
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Weffc++")

# Add main file of project root directory as source file
set(SOURCE_FILES transpile.cpp formatter.cpp formatter.h)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(pysh ${SOURCE_FILES})

install(TARGETS pysh)

# Needed for vim users
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
