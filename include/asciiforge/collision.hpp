#pragma once

#include "asciiforge/map.hpp"

namespace asciiforge {

// Validates whether an entity can occupy grid coordinates (x, y)
bool can_move_to(const TileMap& map, int x, int y) noexcept;

// Checks whether two points on the grid overlap
bool check_overlap(const Point& a, const Point& b) noexcept;

} // namespace asciiforge

