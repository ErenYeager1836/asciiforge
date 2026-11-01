#include <catch2/catch_test_macros.hpp>
#include "asciiforge/collision.hpp"
#include "asciiforge/map.hpp"

using namespace asciiforge;

TEST_CASE("Collision: wall blocks movement and floor allows", "[collision]") {
    const std::string map_data =
        "#####\n"
        "#@..#\n"
        "#####\n";

    MapLoadResult result = MapLoader::from_string(map_data);
    REQUIRE(result.success);
    const TileMap& map = result.map;

    // Floor cells allow movement
    CHECK(can_move_to(map, 1, 1));
    CHECK(can_move_to(map, 2, 1));
    CHECK(can_move_to(map, 3, 1));

    // Wall cells block movement
    CHECK_FALSE(can_move_to(map, 0, 1));
    CHECK_FALSE(can_move_to(map, 4, 1));
    CHECK_FALSE(can_move_to(map, 2, 0));
    CHECK_FALSE(can_move_to(map, 2, 2));
}

TEST_CASE("Collision: map edges block movement outside bounds", "[collision]") {
    const std::string map_data =
        "#####\n"
        "#@..#\n"
        "#####\n";

    MapLoadResult result = MapLoader::from_string(map_data);
    REQUIRE(result.success);
    const TileMap& map = result.map;

    // Negative coordinates
    CHECK_FALSE(can_move_to(map, -1, 1));
    CHECK_FALSE(can_move_to(map, 1, -1));
    CHECK_FALSE(can_move_to(map, -5, -5));

    // Beyond width and height
    CHECK_FALSE(can_move_to(map, 5, 1));
    CHECK_FALSE(can_move_to(map, 1, 3));
    CHECK_FALSE(can_move_to(map, 10, 10));
}

TEST_CASE("Collision: check_overlap correctly identifies point equality", "[collision]") {
    Point a{3, 4};
    Point b{3, 4};
    Point c{3, 5};
    Point d{2, 4};

    CHECK(check_overlap(a, b));
    CHECK_FALSE(check_overlap(a, c));
    CHECK_FALSE(check_overlap(a, d));
}

