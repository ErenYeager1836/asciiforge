#include <catch2/catch_test_macros.hpp>
#include "asciiforge/clock.hpp"
#include "asciiforge/entity.hpp"
#include "asciiforge/game_loop.hpp"
#include "asciiforge/map.hpp"
#include "platform/terminal_null.hpp"

using namespace asciiforge;

TEST_CASE("NullTerminal initialization and lifecycle", "[platform]") {
    NullTerminal terminal(100, 30);

    CHECK_FALSE(terminal.is_initialized());
    CHECK(terminal.get_size() == TerminalSize{100, 30});

    CHECK(terminal.init());
    CHECK(terminal.is_initialized());

    terminal.shutdown();
    CHECK_FALSE(terminal.is_initialized());
}

TEST_CASE("NullTerminal output recording and clearing", "[platform]") {
    NullTerminal terminal;
    terminal.init();

    terminal.write("Hello ");
    terminal.write("World!");
    terminal.flush();

    CHECK(terminal.get_output() == "Hello World!");
    CHECK(terminal.flush_count() == 1);

    terminal.clear_output();
    CHECK(terminal.get_output().empty());
}

TEST_CASE("NullTerminal key queue FIFO delivery and exhaustion", "[platform]") {
    NullTerminal terminal;
    terminal.init();

    CHECK(terminal.poll_key() == Key::None);

    terminal.push_keys({Key::W, Key::A, Key::S, Key::D});

    CHECK(terminal.poll_key() == Key::W);
    CHECK(terminal.poll_key() == Key::A);
    CHECK(terminal.poll_key() == Key::S);
    CHECK(terminal.poll_key() == Key::D);
    CHECK(terminal.poll_key() == Key::None);
}

TEST_CASE("Null terminal key queue feeds player movement", "[platform]") {
    const std::string map_data =
        "#######\n"
        "#@....#\n"
        "#######\n";

    MapLoadResult result = MapLoader::from_string(map_data);
    REQUIRE(result.success);
    const TileMap& map = result.map;

    Player player(map.player_start());
    CHECK(player.position() == Point{1, 1});

    NullTerminal terminal;
    terminal.init();

    // Script key sequence: Right, Right, Down (blocked by wall), Right, Q
    terminal.push_keys({Key::Right, Key::Right, Key::Down, Key::Right, Key::Q});

    auto clock = std::make_shared<MockClock>(0.0);
    GameLoopConfig config;
    config.target_updates_per_second = 10.0;
    GameLoop loop(clock, config);

    // Run game loop steps driven by the NullTerminal input queue
    bool loop_active = true;
    while (loop_active) {
        clock->advance(0.1);
        loop_active = loop.step(
            [&]([[maybe_unused]] double dt) -> bool {
                Key key = terminal.poll_key();
                if (key == Key::None) {
                    return false;
                }
                if (key == Key::Q) {
                    return false; // Exit on Q
                }
                player.handle_key(map, key);
                return true;
            },
            []([[maybe_unused]] double alpha) {}
        );
    }

    // Player started at (1, 1), moved Right to (2, 1), Right to (3, 1), Down blocked, Right to (4, 1)
    CHECK(player.position() == Point{4, 1});
    // Queue should be completely consumed
    CHECK(terminal.poll_key() == Key::None);
}

