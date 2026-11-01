#include "terminal_null.hpp"

namespace asciiforge {

NullTerminal::NullTerminal(int width, int height)
    : size_{width, height} {}

NullTerminal::~NullTerminal() {
    shutdown();
}

bool NullTerminal::init() {
    initialized_ = true;
    return true;
}

void NullTerminal::shutdown() {
    initialized_ = false;
}

bool NullTerminal::is_initialized() const noexcept {
    return initialized_;
}

TerminalSize NullTerminal::get_size() const {
    return size_;
}

void NullTerminal::set_size(int width, int height) {
    size_.width = width;
    size_.height = height;
}

void NullTerminal::write(std::string_view text) {
    output_.append(text.data(), text.size());
}

void NullTerminal::flush() {
    ++flush_count_;
}

Key NullTerminal::poll_key() {
    if (key_queue_.empty()) {
        return Key::None;
    }
    const Key k = key_queue_.front();
    key_queue_.pop();
    return k;
}

void NullTerminal::push_key(Key key) {
    key_queue_.push(key);
}

void NullTerminal::push_keys(std::initializer_list<Key> keys) {
    for (Key k : keys) {
        key_queue_.push(k);
    }
}

const std::string& NullTerminal::get_output() const noexcept {
    return output_;
}

void NullTerminal::clear_output() {
    output_.clear();
}

size_t NullTerminal::flush_count() const noexcept {
    return flush_count_;
}

#ifndef _WIN32
std::unique_ptr<ITerminal> create_platform_terminal() {
    return std::make_unique<NullTerminal>();
}
#endif

} // namespace asciiforge

