set(Boost_INCLUDE_DIR /usr/local/include/)
set(Boost_LIBRARY_DIR /usr/local/lib/)

find_package(Boost REQUIRED)
find_package(Boost COMPONENTS program_options regex REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

add_executable(tests test_transpile.cpp ../src/transpile.cpp ../src/transpile.hpp ../src/formatter.cpp ../src/formatter.hpp)
target_include_directories(tests PRIVATE ../src/)
target_link_libraries(tests ${Boost_LIBS} Boost::regex Boost::program_options Catch2::Catch2WithMain)

target_compile_options(tests PRIVATE --coverage)
target_link_options(tests PRIVATE --coverage)

add_test(NAME tests COMMAND tests)
install(TARGETS tests)

include(CTest)
include(Catch)