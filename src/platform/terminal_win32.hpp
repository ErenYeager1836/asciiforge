#pragma once

#ifdef _WIN32

#include "terminal.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace asciiforge {

class Win32Terminal : public ITerminal {
public:
    Win32Terminal();
    ~Win32Terminal() override;

    bool init() override;
    void shutdown() override;
    bool is_initialized() const noexcept override;

    TerminalSize get_size() const override;
    void write(std::string_view text) override;
    void flush() override;
    Key poll_key() override;

private:
    HANDLE out_handle_{INVALID_HANDLE_VALUE};
    HANDLE in_handle_{INVALID_HANDLE_VALUE};
    DWORD orig_out_mode_{0};
    DWORD orig_in_mode_{0};
    UINT orig_cp_{0};
    bool initialized_{false};
};

} // namespace asciiforge

#endif // _WIN32

