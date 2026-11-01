#pragma once

#include "asciiforge/clock.hpp"

#include <cstdint>
#include <functional>
#include <memory>

namespace asciiforge {

struct GameLoopConfig {
    double target_updates_per_second{60.0};
    double max_frame_time{0.25}; // Clamp to prevent spiral of death
};

class GameLoop {
public:
    using UpdateFn = std::function<bool(double dt)>;
    using RenderFn = std::function<void(double alpha)>;

    explicit GameLoop(std::shared_ptr<IClock> clock = std::make_shared<SteadyClock>(),
                      GameLoopConfig config = {});

    // Runs continuously until update_fn returns false or stop() is invoked
    void run(const UpdateFn& update_fn, const RenderFn& render_fn);

    // Executes a single frame/step. Useful for deterministic tests or external loops.
    // Returns false if update_fn requested termination.
    bool step(const UpdateFn& update_fn, const RenderFn& render_fn);

    void stop() noexcept;

    bool is_running() const noexcept { return running_; }
    uint64_t total_updates() const noexcept { return total_updates_; }
    uint64_t total_renders() const noexcept { return total_renders_; }
    double fps() const noexcept { return current_fps_; }
    double dt() const noexcept { return dt_; }
    double accumulator() const noexcept { return accumulator_; }

    void reset();

private:
    std::shared_ptr<IClock> clock_;
    GameLoopConfig config_;
    double dt_{1.0 / 60.0};
    double current_time_{0.0};
    double accumulator_{0.0};
    bool running_{false};
    bool initialized_time_{false};

    uint64_t total_updates_{0};
    uint64_t total_renders_{0};

    double current_fps_{0.0};
    double fps_timer_{0.0};
    uint64_t frames_in_current_second_{0};
};

} // namespace asciiforge

