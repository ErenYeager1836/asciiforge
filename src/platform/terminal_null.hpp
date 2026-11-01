#pragma once

#include "terminal.hpp"

#include <initializer_list>
#include <queue>
#include <string>

namespace asciiforge {

class NullTerminal : public ITerminal {
public:
    explicit NullTerminal(int width = 80, int height = 25);
    ~NullTerminal() override;

    bool init() override;
    void shutdown() override;
    bool is_initialized() const noexcept override;

    TerminalSize get_size() const override;
    void set_size(int width, int height);

    void write(std::string_view text) override;
    void flush() override;
    Key poll_key() override;

    // Test inspection and injection helpers
    void push_key(Key key);
    void push_keys(std::initializer_list<Key> keys);
    const std::string& get_output() const noexcept;
    void clear_output();
    size_t flush_count() const noexcept;

private:
    TerminalSize size_;
    bool initialized_{false};
    std::queue<Key> key_queue_;
    std::string output_;
    size_t flush_count_{0};
};

} // namespace asciiforge

