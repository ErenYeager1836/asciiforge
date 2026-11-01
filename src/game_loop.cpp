#include "asciiforge/game_loop.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

namespace asciiforge {

GameLoop::GameLoop(std::shared_ptr<IClock> clock, GameLoopConfig config)
    : clock_(clock ? std::move(clock) : std::make_shared<SteadyClock>()),
      config_(config),
      dt_(config.target_updates_per_second > 0.0 ? (1.0 / config.target_updates_per_second) : (1.0 / 60.0)) {}

void GameLoop::reset() {
    current_time_ = 0.0;
    accumulator_ = 0.0;
    running_ = false;
    initialized_time_ = false;
    total_updates_ = 0;
    total_renders_ = 0;
    current_fps_ = 0.0;
    fps_timer_ = 0.0;
    frames_in_current_second_ = 0;
}

void GameLoop::stop() noexcept {
    running_ = false;
}

bool GameLoop::step(const UpdateFn& update_fn, const RenderFn& render_fn) {
    if (!initialized_time_) {
        current_time_ = clock_->now_seconds();
        initialized_time_ = true;
    }

    const double new_time = clock_->now_seconds();
    double frame_time = new_time - current_time_;
    if (frame_time < 0.0) {
        frame_time = 0.0;
    }
    if (frame_time > config_.max_frame_time) {
        frame_time = config_.max_frame_time;
    }
    current_time_ = new_time;
    accumulator_ += frame_time;

    // FPS estimation
    fps_timer_ += frame_time;
    frames_in_current_second_++;
    if (fps_timer_ >= 1.0) {
        current_fps_ = static_cast<double>(frames_in_current_second_) / fps_timer_;
        frames_in_current_second_ = 0;
        fps_timer_ = 0.0;
    }

    // Fixed timestep updates with epsilon tolerance for floating-point precision
    constexpr double epsilon = 1e-9;
    while (accumulator_ + epsilon >= dt_) {
        if (!update_fn(dt_)) {
            running_ = false;
            return false;
        }
        accumulator_ -= dt_;
        total_updates_++;
    }
    if (accumulator_ < 0.0) {
        accumulator_ = 0.0;
    }

    // Render with interpolation factor alpha
    const double alpha = (dt_ > 0.0) ? (accumulator_ / dt_) : 0.0;
    render_fn(alpha);
    total_renders_++;

    return true;
}

void GameLoop::run(const UpdateFn& update_fn, const RenderFn& render_fn) {
    running_ = true;
    while (running_) {
        if (!step(update_fn, render_fn)) {
            break;
        }

        // Small sleep to yield CPU slice when frame finished quickly
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

} // namespace asciiforge

