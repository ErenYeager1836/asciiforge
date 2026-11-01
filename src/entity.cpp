#include "asciiforge/entity.hpp"

namespace asciiforge {

Player::Player(Point start_pos)
    : pos_(start_pos),
      start_pos_(start_pos),
      reset_count_(0) {}

void Player::reset_to_start() noexcept {
    pos_ = start_pos_;
}

bool Player::try_move(const TileMap& map, int dx, int dy) {
    const int target_x = pos_.x + dx;
    const int target_y = pos_.y + dy;

    if (can_move_to(map, target_x, target_y)) {
        pos_.x = target_x;
        pos_.y = target_y;
        return true;
    }
    return false;
}

bool Player::handle_key(const TileMap& map, Key key) {
    switch (key) {
        case Key::Up:
        case Key::W:
            return try_move(map, 0, -1);
        case Key::Down:
        case Key::S:
            return try_move(map, 0, 1);
        case Key::Left:
        case Key::A:
            return try_move(map, -1, 0);
        case Key::Right:
        case Key::D:
            return try_move(map, 1, 0);
        default:
            return false;
    }
}

Enemy::Enemy(Point start_pos, Vec2 velocity)
    : pos_(start_pos),
      start_pos_(start_pos),
      velocity_(velocity) {}

void Enemy::update(const TileMap& map) {
    if (velocity_.x == 0 && velocity_.y == 0) {
        return;
    }

    const int target_x = pos_.x + velocity_.x;
    const int target_y = pos_.y + velocity_.y;

    if (can_move_to(map, target_x, target_y)) {
        pos_.x = target_x;
        pos_.y = target_y;
    } else {
        // Reverse direction
        velocity_.x = -velocity_.x;
        velocity_.y = -velocity_.y;

        const int rev_x = pos_.x + velocity_.x;
        const int rev_y = pos_.y + velocity_.y;

        if (can_move_to(map, rev_x, rev_y)) {
            pos_.x = rev_x;
            pos_.y = rev_y;
        }
    }
}

} // namespace asciiforge

