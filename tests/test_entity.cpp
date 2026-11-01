#include <catch2/catch_test_macros.hpp>
#include "asciiforge/entity.hpp"
#include "asciiforge/map.hpp"

using namespace asciiforge;

TEST_CASE("Player movement: moves on valid floor and is blocked by walls", "[entity]") {
    const std::string map_data =
        "#####\n"
        "#@..#\n"
        "#####\n";

    MapLoadResult result = MapLoader::from_string(map_data);
    REQUIRE(result.success);
    const TileMap& map = result.map;

    Player player(map.player_start());
    CHECK(player.position() == Point{1, 1});

    // Try moving left into wall at (0, 1)
    CHECK_FALSE(player.try_move(map, -1, 0));
    CHECK(player.position() == Point{1, 1});

    // Try moving up into wall at (1, 0)
    CHECK_FALSE(player.try_move(map, 0, -1));
    CHECK(player.position() == Point{1, 1});

    // Move right into floor at (2, 1)
    CHECK(player.try_move(map, 1, 0));
    CHECK(player.position() == Point{2, 1});

    // Move right into floor at (3, 1)
    CHECK(player.try_move(map, 1, 0));
    CHECK(player.position() == Point{3, 1});

    // Try moving right into wall at (4, 1)
    CHECK_FALSE(player.try_move(map, 1, 0));
    CHECK(player.position() == Point{3, 1});
}

TEST_CASE("Player input handling: WASD and arrow keys", "[entity]") {
    const std::string map_data =
        "#####\n"
        "#...#\n"
        "#.@.#\n"
        "#...#\n"
        "#####\n";

    MapLoadResult result = MapLoader::from_string(map_data);
    REQUIRE(result.success);
    const TileMap& map = result.map;

    Player player(map.player_start()); // at (2, 2)

    CHECK(player.handle_key(map, Key::Up));
    CHECK(player.position() == Point{2, 1});

    CHECK(player.handle_key(map, Key::Down));
    CHECK(player.position() == Point{2, 2});

    CHECK(player.handle_key(map, Key::A));
    CHECK(player.position() == Point{1, 2});

    CHECK(player.handle_key(map, Key::D));
    CHECK(player.position() == Point{2, 2});

    CHECK(player.handle_key(map, Key::W));
    CHECK(player.position() == Point{2, 1});

    CHECK(player.handle_key(map, Key::S));
    CHECK(player.position() == Point{2, 2});
}

TEST_CASE("Player reset_to_start returns to spawn point", "[entity]") {
    const std::string map_data =
        "#######\n"
        "#@....#\n"
        "#######\n";

    MapLoadResult result = MapLoader::from_string(map_data);
    REQUIRE(result.success);
    const TileMap& map = result.map;

    Player player(map.player_start());
    player.try_move(map, 1, 0);
    player.try_move(map, 1, 0);
    player.try_move(map, 1, 0);
    CHECK(player.position() == Point{4, 1});

    player.reset_to_start();
    player.increment_resets();
    CHECK(player.position() == Point{1, 1});
    CHECK(player.reset_count() == 1);
}

TEST_CASE("Enemy patrol reverses horizontal direction at walls", "[entity]") {
    // 5 wide corridor: # (0), E (1), . (2), . (3), # (4)
    const std::string map_data =
        "#####\n"
        "#E..#\n"
        "#####\n";

    MapLoadResult result = MapLoader::from_string(map_data);
    // Add enemy start manually if no '@' in raw string
    std::string valid_map =
        "######\n"
        "#@.E.#\n"
        "######\n";

    result = MapLoader::from_string(valid_map);
    REQUIRE(result.success);
    const TileMap& map = result.map;

    // Enemy starts at (3, 1), moving right with velocity (1, 0)
    Enemy enemy(Point{3, 1}, Vec2{1, 0});

    // Step 1: moves to (4, 1) which is floor
    enemy.update(map);
    CHECK(enemy.position() == Point{4, 1});
    CHECK(enemy.velocity() == Vec2{1, 0});

    // Step 2: (5, 1) is a wall (#). Enemy should reverse velocity to (-1, 0) and move to (3, 1)
    enemy.update(map);
    CHECK(enemy.velocity() == Vec2{-1, 0});
    CHECK(enemy.position() == Point{3, 1});

    // Step 3: moves left to (2, 1)
    enemy.update(map);
    CHECK(enemy.position() == Point{2, 1});
    CHECK(enemy.velocity() == Vec2{-1, 0});
}

TEST_CASE("Enemy patrol reverses vertical direction at walls", "[entity]") {
    const std::string map_data =
        "###\n"
        "#@#\n"
        "#E#\n"
        "#.#\n"
        "###\n";

    MapLoadResult result = MapLoader::from_string(map_data);
    REQUIRE(result.success);
    const TileMap& map = result.map;

    // Enemy at (1, 2) moving down (0, 1)
    Enemy enemy(Point{1, 2}, Vec2{0, 1});

    // Step 1: moves down to (1, 3) which is floor
    enemy.update(map);
    CHECK(enemy.position() == Point{1, 3});
    CHECK(enemy.velocity() == Vec2{0, 1});

    // Step 2: (1, 4) is wall. Reverse velocity to (0, -1) and move to (1, 2)
    enemy.update(map);
    CHECK(enemy.velocity() == Vec2{0, -1});
    CHECK(enemy.position() == Point{1, 2});
}

