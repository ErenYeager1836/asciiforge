#include "asciiforge/map.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace asciiforge {

TileMap::TileMap(int width, int height, std::vector<TileType> tiles, Point player_start, std::vector<Point> enemy_starts)
    : width_(width),
      height_(height),
      tiles_(std::move(tiles)),
      player_start_(player_start),
      enemy_starts_(std::move(enemy_starts)) {}

TileType TileMap::get(int x, int y) const {
    if (!in_bounds(x, y)) {
        throw std::out_of_range("TileMap::get: coordinates out of bounds");
    }
    return tiles_[static_cast<size_t>(y * width_ + x)];
}

bool TileMap::is_wall(int x, int y) const {
    if (!in_bounds(x, y)) {
        return true; // Out of bounds is treated as solid wall
    }
    return tiles_[static_cast<size_t>(y * width_ + x)] == TileType::Wall;
}

bool TileMap::is_floor(int x, int y) const {
    if (!in_bounds(x, y)) {
        return false;
    }
    return tiles_[static_cast<size_t>(y * width_ + x)] == TileType::Floor;
}

MapLoadResult MapLoader::from_string(std::string_view content) {
    MapLoadResult result;

    if (content.empty()) {
        result.error = "Map content is empty";
        return result;
    }

    // Split content into lines, handling both \n and \r\n
    std::vector<std::string> lines;
    std::string current_line;
    for (char c : content) {
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            lines.push_back(current_line);
            current_line.clear();
        } else {
            current_line.push_back(c);
        }
    }
    if (!current_line.empty() || (!lines.empty() && content.back() != '\n')) {
        lines.push_back(current_line);
    }

    // Remove empty trailing lines
    while (!lines.empty() && lines.back().empty()) {
        lines.pop_back();
    }

    if (lines.empty()) {
        result.error = "Map content is empty";
        return result;
    }

    const size_t expected_width = lines[0].size();
    if (expected_width == 0) {
        result.error = "Map content is empty";
        return result;
    }

    // Check for ragged rows
    for (size_t row = 0; row < lines.size(); ++row) {
        if (lines[row].size() != expected_width) {
            result.error_line = static_cast<int>(row + 1);
            result.error_column = static_cast<int>(std::min(lines[row].size(), expected_width) + 1);
            result.error = "Ragged row at line " + std::to_string(row + 1) +
                           ": expected width " + std::to_string(expected_width) +
                           ", got " + std::to_string(lines[row].size());
            return result;
        }
    }

    const int width = static_cast<int>(expected_width);
    const int height = static_cast<int>(lines.size());
    std::vector<TileType> tiles;
    tiles.reserve(static_cast<size_t>(width * height));

    Point player_start{-1, -1};
    bool has_player_start = false;
    std::vector<Point> enemy_starts;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const char ch = lines[static_cast<size_t>(y)][static_cast<size_t>(x)];
            switch (ch) {
                case '#':
                    tiles.push_back(TileType::Wall);
                    break;
                case '.':
                    tiles.push_back(TileType::Floor);
                    break;
                case '@':
                    if (has_player_start) {
                        result.error_line = y + 1;
                        result.error_column = x + 1;
                        result.error = "Multiple player start positions '@' found at line " +
                                       std::to_string(y + 1) + ", column " + std::to_string(x + 1);
                        return result;
                    }
                    player_start = Point{x, y};
                    has_player_start = true;
                    tiles.push_back(TileType::Floor);
                    break;
                case 'E':
                    enemy_starts.push_back(Point{x, y});
                    tiles.push_back(TileType::Floor);
                    break;
                default:
                    result.error_line = y + 1;
                    result.error_column = x + 1;
                    result.error = "Unknown character '" + std::string(1, ch) +
                                   "' at line " + std::to_string(y + 1) +
                                   ", column " + std::to_string(x + 1);
                    return result;
            }
        }
    }

    if (!has_player_start) {
        result.error = "Map is missing player start '@'";
        return result;
    }

    result.success = true;
    result.map = TileMap(width, height, std::move(tiles), player_start, std::move(enemy_starts));
    return result;
}

MapLoadResult MapLoader::from_file(const std::filesystem::path& path) {
    MapLoadResult result;
    std::ifstream file(path);
    if (!file.is_open()) {
        result.error = "Failed to open map file: " + path.string();
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return from_string(buffer.str());
}

} // namespace asciiforge

