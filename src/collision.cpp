#include "asciiforge/collision.hpp"

namespace asciiforge {

bool can_move_to(const TileMap& map, int x, int y) noexcept {
    if (!map.in_bounds(x, y)) {
        return false;
    }
    return !map.is_wall(x, y);
}

bool check_overlap(const Point& a, const Point& b) noexcept {
    return a == b;
}

} // namespace asciiforge

