cmake_minimum_required(VERSION 3.13)
project(pysh)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Weffc++ -fprofile-arcs -ftest-coverage --coverage")
set(Boost_INCLUDE_DIR /usr/local/include/)
set(Boost_LIBRARY_DIR /usr/local/lib/)

Include(FetchContent)
FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(catch2)

find_package(Boost REQUIRED)
find_package(Boost COMPONENTS program_options regex REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

# Add main file of project root directory as source file
set(SOURCE_FILES src/transpile.cpp src/formatter.cpp src/formatter.hpp src/transpile.hpp src/main.cpp src/main.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(pysh ${SOURCE_FILES})
target_link_libraries(pysh ${Boost_LIBS} Boost::regex Boost::program_options)
target_include_directories(pysh PRIVATE src/)

install(TARGETS pysh)

# Needed for vim users
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

enable_testing()
add_subdirectory(tests)
