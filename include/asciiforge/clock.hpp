#pragma once

#include <chrono>

namespace asciiforge {

class IClock {
public:
    virtual ~IClock() = default;
    virtual double now_seconds() const = 0;
};

class SteadyClock : public IClock {
public:
    double now_seconds() const override {
        using namespace std::chrono;
        const auto now = steady_clock::now();
        return duration<double>(now.time_since_epoch()).count();
    }
};

class MockClock : public IClock {
public:
    explicit MockClock(double start_time = 0.0) noexcept : time_(start_time) {}

    double now_seconds() const override { return time_; }

    void advance(double seconds) noexcept {
        if (seconds > 0.0) {
            time_ += seconds;
        }
    }

    void set(double seconds) noexcept {
        time_ = seconds;
    }

private:
    double time_{0.0};
};

} // namespace asciiforge

