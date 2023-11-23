#include <catch2/catch_test_macros.hpp>
#include "formatter.hpp"

TEST_CASE("custom formatter works correctly", "[formatter]")
{
    type_formatter formatter("custom", 0);
    REQUIRE(formatter.format() == "try:\n    _ = custom(_)\nexcept NameError:\n    raise NameError('custom is not a valid custom formatter.')\nexcept TypeError:\n    raise TypeError('custom is not callable, or takes too many args.')\n");
}