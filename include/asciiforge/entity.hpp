#pragma once

#include "asciiforge/collision.hpp"
#include "asciiforge/map.hpp"
#include "asciiforge/terminal.hpp"

namespace asciiforge {

struct Vec2 {
    int x{0};
    int y{0};

    bool operator==(const Vec2& o) const noexcept { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const noexcept { return !(*this == o); }
};

class Player {
public:
    explicit Player(Point start_pos = {0, 0});

    Point position() const noexcept { return pos_; }
    Point start_position() const noexcept { return start_pos_; }
    void set_position(Point p) noexcept { pos_ = p; }
    void reset_to_start() noexcept;

    // Attempts to move by (dx, dy) with collision against map
    bool try_move(const TileMap& map, int dx, int dy);

    // Processes an input key and attempts movement if directional
    bool handle_key(const TileMap& map, Key key);

    int reset_count() const noexcept { return reset_count_; }
    void increment_resets() noexcept { reset_count_++; }

private:
    Point pos_{0, 0};
    Point start_pos_{0, 0};
    int reset_count_{0};
};

class Enemy {
public:
    explicit Enemy(Point start_pos = {0, 0}, Vec2 velocity = {1, 0});

    Point position() const noexcept { return pos_; }
    Point start_position() const noexcept { return start_pos_; }
    Vec2 velocity() const noexcept { return velocity_; }
    void set_velocity(Vec2 vel) noexcept { velocity_ = vel; }

    // Patrol update: moves along velocity. If blocked by wall or map bounds,
    // reverses velocity and attempts movement in reverse direction.
    void update(const TileMap& map);

private:
    Point pos_{0, 0};
    Point start_pos_{0, 0};
    Vec2 velocity_{1, 0};
};

} // namespace asciiforge

