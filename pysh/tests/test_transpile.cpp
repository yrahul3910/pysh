#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "transpile.hpp"

TEST_CASE("pure Python line is not modified", "[transpile]")
{
    std::string line = "print('Hello, world!')";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "print('Hello, world!')\n");
}
