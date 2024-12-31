cmake_minimum_required(VERSION 3.13)
project(pysh)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

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

add_library(
    common
    lib/formatter.cpp
    lib/transpile.cpp
)
target_link_libraries(common ${Boost_LIBS} Boost::regex Boost::program_options)
target_include_directories(common PUBLIC include)

# Add main file of project root directory as source file
set(PYSH_SOURCE_FILES src/main.cpp)
set(PYSHTEST_SOURCE_FILES src/pyshtest.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(pysh ${PYSH_SOURCE_FILES})
target_link_libraries(pysh ${Boost_LIBS} Boost::regex Boost::program_options common)
target_include_directories(pysh PRIVATE src/)

add_executable(pyshtest ${PYSHTEST_SOURCE_FILES})
target_link_libraries(pyshtest ${Boost_LIBS} Boost::regex Boost::program_options common)
target_include_directories(pyshtest PRIVATE src/)

install(TARGETS pysh pyshtest)

# Needed for vim users
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
add_subdirectory(tests)
