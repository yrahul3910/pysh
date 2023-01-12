cmake_minimum_required(VERSION 3.13)  # CMake version check
project(pysh)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 20)            # Enable c++20 standard
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Weffc++")
set(Boost_INCLUDE_DIR /usr/local/include/)
set(Boost_LIBRARY_DIR /usr/local/lib/)

find_package(Boost REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

# Add main file of project root directory as source file
set(SOURCE_FILES transpile.cpp formatter.cpp formatter.h)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(pysh ${SOURCE_FILES})
target_link_libraries(pysh ${Boost_LIBS} Boost::program_options)

install(TARGETS pysh)

# Needed for vim users
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
