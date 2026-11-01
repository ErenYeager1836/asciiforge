#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace asciiforge {

enum class TileType : uint8_t {
    Floor,
    Wall
};

struct Point {
    int x{0};
    int y{0};

    bool operator==(const Point& o) const noexcept { return x == o.x && y == o.y; }
    bool operator!=(const Point& o) const noexcept { return !(*this == o); }
};

class TileMap {
public:
    TileMap() = default;
    TileMap(int width, int height, std::vector<TileType> tiles, Point player_start, std::vector<Point> enemy_starts);

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    bool in_bounds(int x, int y) const noexcept {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    TileType get(int x, int y) const;
    bool is_wall(int x, int y) const;
    bool is_floor(int x, int y) const;

    Point player_start() const noexcept { return player_start_; }
    const std::vector<Point>& enemy_starts() const noexcept { return enemy_starts_; }

private:
    int width_{0};
    int height_{0};
    std::vector<TileType> tiles_;
    Point player_start_{0, 0};
    std::vector<Point> enemy_starts_;
};

struct MapLoadResult {
    bool success{false};
    std::string error;
    int error_line{-1};     // 1-based line number on error
    int error_column{-1};   // 1-based column number on error
    TileMap map;
};

class MapLoader {
public:
    static MapLoadResult from_string(std::string_view content);
    static MapLoadResult from_file(const std::filesystem::path& path);
};

} // namespace asciiforge

