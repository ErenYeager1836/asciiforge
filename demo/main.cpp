#include "asciiforge/buffer.hpp"
#include "asciiforge/collision.hpp"
#include "asciiforge/entity.hpp"
#include "asciiforge/game_loop.hpp"
#include "asciiforge/map.hpp"
#include "asciiforge/terminal.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static const char* const EMBEDDED_ROOM =
    "########################################\n"
    "#@........#............................#\n"
    "#.........#.........E..................#\n"
    "#.........#............................#\n"
    "#.........#...........##################\n"
    "#.........#...........#................#\n"
    "#.........#############................#\n"
    "#......................................#\n"
    "#......................................#\n"
    "#..................E...................#\n"
    "#......................................#\n"
    "########################################\n";

int main(int argc, char* argv[]) {
    asciiforge::MapLoadResult map_result;

    // Check multiple candidate locations for room1.txt
    std::vector<fs::path> candidate_paths = {
        "maps/room1.txt",
        "demo/maps/room1.txt",
        "../demo/maps/room1.txt",
        "../../demo/maps/room1.txt"
    };

    if (argc > 1) {
        candidate_paths.insert(candidate_paths.begin(), argv[1]);
    }

    bool loaded_from_file = false;
    for (const auto& path : candidate_paths) {
        if (fs::exists(path)) {
            map_result = asciiforge::MapLoader::from_file(path);
            if (map_result.success) {
                loaded_from_file = true;
                break;
            }
        }
    }

    if (!loaded_from_file) {
        // Fallback to embedded default map
        map_result = asciiforge::MapLoader::from_string(EMBEDDED_ROOM);
    }

    if (!map_result.success) {
        std::cerr << "Failed to load map: " << map_result.error << std::endl;
        return 1;
    }

    const asciiforge::TileMap& map = map_result.map;

    auto terminal = asciiforge::create_platform_terminal();
    if (!terminal->init()) {
        std::cerr << "Failed to initialize terminal." << std::endl;
        return 1;
    }

    // Clear terminal screen and reset cursor at launch
    terminal->write("\033[2J\033[H");
    terminal->flush();

    asciiforge::Player player(map.player_start());
    std::vector<asciiforge::Enemy> enemies;
    for (const auto& start : map.enemy_starts()) {
        enemies.emplace_back(start, asciiforge::Vec2{1, 0});
    }

    const int buffer_w = std::max(map.width(), 80);
    const int buffer_h = map.height() + 3; // Extra rows for HUD and status messages

    asciiforge::ScreenBuffer current_buffer(buffer_w, buffer_h);
    asciiforge::ScreenBuffer previous_buffer(buffer_w, buffer_h);
    bool first_frame = true;

    std::string status_msg = "Welcome to asciiforge! Use WASD or Arrows to move. Q to quit.";
    double status_timer = 3.0;
    int enemy_tick_counter = 0;

    asciiforge::GameLoop loop;

    loop.run(
        [&](double dt) -> bool {
            // Process all pending input
            while (true) {
                const asciiforge::Key key = terminal->poll_key();
                if (key == asciiforge::Key::None) {
                    break;
                }
                if (key == asciiforge::Key::Q || key == asciiforge::Key::Escape) {
                    return false; // Exit game loop
                }
                player.handle_key(map, key);
            }

            // Update status message timer
            if (status_timer > 0.0) {
                status_timer -= dt;
                if (status_timer <= 0.0) {
                    status_msg.clear();
                }
            }

            // Move enemies at 4 steps per second (every 15 ticks at 60Hz)
            ++enemy_tick_counter;
            if (enemy_tick_counter >= 15) {
                enemy_tick_counter = 0;
                for (auto& enemy : enemies) {
                    enemy.update(map);
                }
            }

            // Check collision between player and enemies
            for (const auto& enemy : enemies) {
                if (asciiforge::check_overlap(player.position(), enemy.position())) {
                    player.reset_to_start();
                    player.increment_resets();
                    status_msg = "OUCH! Caught by an enemy! Reset to start.";
                    status_timer = 2.0;
                    break;
                }
            }

            return true;
        },
        [&]([[maybe_unused]] double alpha) {
            current_buffer.clear(asciiforge::Cell{' ', asciiforge::Color::Default, asciiforge::Color::Default});

            // Draw map
            for (int y = 0; y < map.height(); ++y) {
                for (int x = 0; x < map.width(); ++x) {
                    if (map.is_wall(x, y)) {
                        current_buffer.set(x, y, '#', asciiforge::Color::BrightBlue);
                    } else {
                        current_buffer.set(x, y, '.', asciiforge::Color::BrightBlack);
                    }
                }
            }

            // Draw enemies
            for (const auto& enemy : enemies) {
                current_buffer.set(enemy.position().x, enemy.position().y, 'E', asciiforge::Color::BrightRed);
            }

            // Draw player
            current_buffer.set(player.position().x, player.position().y, '@', asciiforge::Color::BrightYellow);

            // Draw HUD
            std::ostringstream hud;
            hud << "Pos: (" << player.position().x << "," << player.position().y << ")"
                << " | Enemies: " << enemies.size()
                << " | Resets: " << player.reset_count()
                << " | FPS: " << std::fixed << std::setprecision(1) << loop.fps()
                << " | [WASD/Arrows] Move [Q] Quit";

            current_buffer.write_string(0, map.height(), hud.str(), asciiforge::Color::BrightWhite);

            if (!status_msg.empty()) {
                current_buffer.write_string(0, map.height() + 1, status_msg, asciiforge::Color::BrightYellow);
            }

            // Compute diff and render ANSI updates
            const auto diff = asciiforge::compute_diff(current_buffer, previous_buffer, first_frame);
            first_frame = false;

            const std::string ansi = asciiforge::render_diff_to_ansi(diff);
            if (!ansi.empty()) {
                terminal->write(ansi);
                terminal->flush();
            }

            previous_buffer = current_buffer;
        });

    terminal->shutdown();
    std::cout << "\nThanks for playing asciiforge!\n";
    return 0;
}

