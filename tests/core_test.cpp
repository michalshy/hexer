#include <catch2/catch_test_macros.hpp>

#include "core.hpp"

TEST_CASE("version is not empty", "[core]") {
    REQUIRE_FALSE(hexer::version().empty());
}
