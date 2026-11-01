#include <catch2/catch_test_macros.hpp>
#include "asciiforge/clock.hpp"
#include "asciiforge/game_loop.hpp"
#include <cmath>

using namespace asciiforge;

TEST_CASE("Game loop: fixed timestep produces the expected update count for a given elapsed time", "[game_loop]") {
    auto mock_clock = std::make_shared<MockClock>(0.0);
    GameLoopConfig config;
    config.target_updates_per_second = 60.0; // dt = 1/60 ~ 0.016667s
    config.max_frame_time = 2.0; // Allow large single delta for this mathematical test

    GameLoop loop(mock_clock, config);

    int update_count = 0;
    int render_count = 0;

    auto update_fn = [&](double dt) -> bool {
        CHECK(std::abs(dt - 1.0 / 60.0) < 1e-6);
        update_count++;
        return true;
    };

    auto render_fn = [&](double alpha) {
        (void)alpha;
        render_count++;
    };

    // Step 0: initialization step (dt=0)
    loop.step(update_fn, render_fn);
    CHECK(update_count == 0);
    CHECK(render_count == 1);

    // Advance clock by 0.5 seconds -> expected 30 updates at 60Hz
    mock_clock->advance(0.5);
    loop.step(update_fn, render_fn);

    CHECK(update_count == 30);
    CHECK(render_count == 2);
    CHECK(loop.total_updates() == 30);
    CHECK(loop.total_renders() == 2);

    // Advance clock by 1.0 second -> expected 60 more updates -> total 90 updates
    mock_clock->advance(1.0);
    loop.step(update_fn, render_fn);

    CHECK(update_count == 90);
    CHECK(render_count == 3);
}

TEST_CASE("Game loop: accumulator preserves sub-dt remainder across multiple steps", "[game_loop]") {
    auto mock_clock = std::make_shared<MockClock>(0.0);
    GameLoopConfig config;
    config.target_updates_per_second = 10.0; // dt = 0.1s

    GameLoop loop(mock_clock, config);

    int update_count = 0;
    auto update_fn = [&]([[maybe_unused]] double dt) -> bool {
        update_count++;
        return true;
    };
    auto render_fn = []([[maybe_unused]] double alpha) {};

    loop.step(update_fn, render_fn); // Init

    // Advance by 0.06s (less than 0.1s)
    mock_clock->advance(0.06);
    loop.step(update_fn, render_fn);
    CHECK(update_count == 0);
    CHECK(std::abs(loop.accumulator() - 0.06) < 1e-4);

    // Advance by another 0.06s (total 0.12s) -> 1 update, 0.02s remainder
    mock_clock->advance(0.06);
    loop.step(update_fn, render_fn);
    CHECK(update_count == 1);
    CHECK(std::abs(loop.accumulator() - 0.02) < 1e-4);
}

TEST_CASE("Game loop: clamps frame_time to max_frame_time to prevent spiral of death", "[game_loop]") {
    auto mock_clock = std::make_shared<MockClock>(0.0);
    GameLoopConfig config;
    config.target_updates_per_second = 100.0; // dt = 0.01s
    config.max_frame_time = 0.1;              // max 10 updates per frame even if lag is huge

    GameLoop loop(mock_clock, config);

    int update_count = 0;
    auto update_fn = [&]([[maybe_unused]] double dt) -> bool {
        update_count++;
        return true;
    };
    auto render_fn = []([[maybe_unused]] double alpha) {};

    loop.step(update_fn, render_fn); // Init

    // Simulate huge lag of 10.0 seconds
    mock_clock->advance(10.0);
    loop.step(update_fn, render_fn);

    // Because max_frame_time is 0.1s, at dt=0.01s only 10 updates should occur!
    CHECK(update_count == 10);
}

TEST_CASE("Game loop: update returning false immediately terminates execution", "[game_loop]") {
    auto mock_clock = std::make_shared<MockClock>(0.0);
    GameLoopConfig config;
    config.target_updates_per_second = 10.0;

    GameLoop loop(mock_clock, config);

    int update_calls = 0;
    auto update_fn = [&]([[maybe_unused]] double dt) -> bool {
        update_calls++;
        if (update_calls == 2) {
            return false; // Request quit
        }
        return true;
    };
    auto render_fn = []([[maybe_unused]] double alpha) {};

    loop.step(update_fn, render_fn); // Init

    // Advance by 1.0s (would normally be 10 updates)
    mock_clock->advance(1.0);
    bool continue_running = loop.step(update_fn, render_fn);

    CHECK_FALSE(continue_running);
    CHECK(update_calls == 2);
}

