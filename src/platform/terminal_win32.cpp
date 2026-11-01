#ifdef _WIN32

#include "terminal_win32.hpp"

#include <iostream>

namespace asciiforge {

Win32Terminal::Win32Terminal() = default;

Win32Terminal::~Win32Terminal() {
    shutdown();
}

bool Win32Terminal::init() {
    if (initialized_) {
        return true;
    }

    out_handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
    in_handle_ = GetStdHandle(STD_INPUT_HANDLE);

    if (out_handle_ == INVALID_HANDLE_VALUE || in_handle_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    if (!GetConsoleMode(out_handle_, &orig_out_mode_) ||
        !GetConsoleMode(in_handle_, &orig_in_mode_)) {
        return false;
    }

    orig_cp_ = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);

    // Enable virtual terminal processing for ANSI escape sequences
    DWORD out_mode = orig_out_mode_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
    SetConsoleMode(out_handle_, out_mode);

    // Disable line input and echo for raw, non-blocking input
    DWORD in_mode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT;
    SetConsoleMode(in_handle_, in_mode);

    initialized_ = true;

    // Hide cursor initially
    write("\033[?25l");
    flush();

    return true;
}

void Win32Terminal::shutdown() {
    if (!initialized_) {
        return;
    }

    // Restore cursor and reset styling
    write("\033[?25h\033[0m");
    flush();

    if (out_handle_ != INVALID_HANDLE_VALUE) {
        SetConsoleMode(out_handle_, orig_out_mode_);
    }
    if (in_handle_ != INVALID_HANDLE_VALUE) {
        SetConsoleMode(in_handle_, orig_in_mode_);
    }
    if (orig_cp_ != 0) {
        SetConsoleOutputCP(orig_cp_);
    }

    initialized_ = false;
}

bool Win32Terminal::is_initialized() const noexcept {
    return initialized_;
}

TerminalSize Win32Terminal::get_size() const {
    if (out_handle_ == INVALID_HANDLE_VALUE) {
        return TerminalSize{80, 25};
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(out_handle_, &csbi)) {
        const int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        const int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return TerminalSize{width > 0 ? width : 80, height > 0 ? height : 25};
    }
    return TerminalSize{80, 25};
}

void Win32Terminal::write(std::string_view text) {
    if (out_handle_ == INVALID_HANDLE_VALUE || text.empty()) {
        return;
    }
    DWORD written = 0;
    WriteConsoleA(out_handle_, text.data(), static_cast<DWORD>(text.size()), &written, nullptr);
}

void Win32Terminal::flush() {
    std::fflush(stdout);
}

Key Win32Terminal::poll_key() {
    if (!initialized_ || in_handle_ == INVALID_HANDLE_VALUE) {
        return Key::None;
    }

    DWORD num_events = 0;
    if (!GetNumberOfConsoleInputEvents(in_handle_, &num_events) || num_events == 0) {
        return Key::None;
    }

    INPUT_RECORD record;
    DWORD read_events = 0;

    while (num_events > 0) {
        if (!ReadConsoleInputW(in_handle_, &record, 1, &read_events) || read_events == 0) {
            break;
        }
        --num_events;

        if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
            const WORD vk = record.Event.KeyEvent.wVirtualKeyCode;
            const WCHAR uchar = record.Event.KeyEvent.uChar.UnicodeChar;

            switch (vk) {
                case VK_UP:
                    return Key::Up;
                case VK_DOWN:
                    return Key::Down;
                case VK_LEFT:
                    return Key::Left;
                case VK_RIGHT:
                    return Key::Right;
                case VK_ESCAPE:
                    return Key::Escape;
                case VK_RETURN:
                    return Key::Enter;
                case VK_SPACE:
                    return Key::Space;
                default:
                    break;
            }

            if (uchar == L'w' || uchar == L'W') {
                return Key::W;
            }
            if (uchar == L'a' || uchar == L'A') {
                return Key::A;
            }
            if (uchar == L's' || uchar == L'S') {
                return Key::S;
            }
            if (uchar == L'd' || uchar == L'D') {
                return Key::D;
            }
            if (uchar == L'q' || uchar == L'Q') {
                return Key::Q;
            }

            return Key::Other;
        }
    }

    return Key::None;
}

std::unique_ptr<ITerminal> create_platform_terminal() {
    return std::make_unique<Win32Terminal>();
}

} // namespace asciiforge

#endif // _WIN32

