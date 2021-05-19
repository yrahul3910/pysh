cmake_minimum_required(VERSION 3.13)  # CMake version check
project(pysh)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)            # Enable c++14 standard

# Add main file of project root directory as source file
set(SOURCE_FILES transpile.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(pysh ${SOURCE_FILES})

# Needed for vim users
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
