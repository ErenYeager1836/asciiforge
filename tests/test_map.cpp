#include <catch2/catch_test_macros.hpp>
#include "asciiforge/map.hpp"

using namespace asciiforge;

TEST_CASE("Map loader: loads valid map with walls, floor, player, and enemies", "[map]") {
    const std::string map_data =
        "#####\n"
        "#@..#\n"
        "#.E.#\n"
        "#####\n";

    MapLoadResult result = MapLoader::from_string(map_data);

    REQUIRE(result.success);
    CHECK(result.error.empty());
    CHECK(result.map.width() == 5);
    CHECK(result.map.height() == 4);

    CHECK(result.map.player_start() == Point{1, 1});
    REQUIRE(result.map.enemy_starts().size() == 1);
    CHECK(result.map.enemy_starts()[0] == Point{2, 2});

    // Check tiles: borders are walls
    CHECK(result.map.is_wall(0, 0));
    CHECK(result.map.is_wall(4, 0));
    CHECK(result.map.is_wall(0, 3));
    CHECK(result.map.is_wall(4, 3));

    // Player and enemy positions replaced with floor underneath
    CHECK(result.map.is_floor(1, 1));
    CHECK(result.map.is_floor(2, 2));
    CHECK(result.map.is_floor(2, 1));
}

TEST_CASE("Map loader: ragged rows error reports line and column", "[map]") {
    const std::string ragged_map =
        "#####\n"
        "#@..#\n"
        "#.E\n"  // 3 chars instead of 5
        "#####\n";

    MapLoadResult result = MapLoader::from_string(ragged_map);

    CHECK_FALSE(result.success);
    CHECK(result.error_line == 3);
    CHECK(result.error.find("Ragged row at line 3") != std::string::npos);
    CHECK(result.error.find("expected width 5, got 3") != std::string::npos);
}

TEST_CASE("Map loader: unknown character error reports line, column, and character", "[map]") {
    const std::string bad_char_map =
        "#####\n"
        "#@?X#\n"
        "#####\n";

    MapLoadResult result = MapLoader::from_string(bad_char_map);

    CHECK_FALSE(result.success);
    CHECK(result.error_line == 2);
    CHECK(result.error_column == 3);
    CHECK(result.error.find("Unknown character '?'") != std::string::npos);
}

TEST_CASE("Map loader: missing player start error", "[map]") {
    const std::string no_player =
        "#####\n"
        "#...#\n"
        "#...#\n"
        "#####\n";

    MapLoadResult result = MapLoader::from_string(no_player);

    CHECK_FALSE(result.success);
    CHECK(result.error.find("missing player start '@'") != std::string::npos);
}

TEST_CASE("Map loader: multiple player starts error", "[map]") {
    const std::string two_players =
        "#####\n"
        "#@..#\n"
        "#..@#\n"
        "#####\n";

    MapLoadResult result = MapLoader::from_string(two_players);

    CHECK_FALSE(result.success);
    CHECK(result.error.find("Multiple player start positions") != std::string::npos);
}

TEST_CASE("Map loader: empty string or blank map reports error", "[map]") {
    MapLoadResult res1 = MapLoader::from_string("");
    CHECK_FALSE(res1.success);
    CHECK(res1.error.find("empty") != std::string::npos);

    MapLoadResult res2 = MapLoader::from_string("   \n\n  \r\n");
    CHECK_FALSE(res2.success);
}

TEST_CASE("Map loader: nonexistent file returns clean error", "[map]") {
    MapLoadResult result = MapLoader::from_file("non_existent_map_file_12345.txt");
    CHECK_FALSE(result.success);
    CHECK(result.error.find("Failed to open map file") != std::string::npos);
}

