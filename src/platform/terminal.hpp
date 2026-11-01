#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

namespace asciiforge {

enum class Key : uint8_t {
  None = 0,
  Up,
  Down,
  Left,
  Right,
  W,
  A,
  S,
  D,
  Q,
  Escape,
  Enter,
  Space,
  Other
};

struct TerminalSize {
  int width{80};
  int height{25};

  bool operator==(const TerminalSize &o) const noexcept {
    return width == o.width && height == o.height;
  }
  bool operator!=(const TerminalSize &o) const noexcept {
    return !(*this == o);
  }
};

class ITerminal {
public:
  virtual ~ITerminal() = default;

  virtual bool init() = 0;
  virtual void shutdown() = 0;
  virtual bool is_initialized() const noexcept = 0;

  virtual TerminalSize get_size() const = 0;
  virtual void write(std::string_view text) = 0;
  virtual void flush() = 0;
  virtual Key poll_key() = 0;
};

// Platform terminal factory
std::unique_ptr<ITerminal> create_platform_terminal();

} // namespace asciiforge
