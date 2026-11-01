#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace asciiforge {

enum class Color : uint8_t {
    Black = 0,
    Red = 1,
    Green = 2,
    Yellow = 3,
    Blue = 4,
    Magenta = 5,
    Cyan = 6,
    White = 7,
    BrightBlack = 8,
    BrightRed = 9,
    BrightGreen = 10,
    BrightYellow = 11,
    BrightBlue = 12,
    BrightMagenta = 13,
    BrightCyan = 14,
    BrightWhite = 15,
    Default = 16
};

// Returns standard ANSI foreground code for the given 16-color palette
int ansi_fg_code(Color color) noexcept;
int ansi_bg_code(Color color) noexcept;

struct Cell {
    char ch{' '};
    Color fg{Color::Default};
    Color bg{Color::Default};

    bool operator==(const Cell& other) const noexcept {
        return ch == other.ch && fg == other.fg && bg == other.bg;
    }
    bool operator!=(const Cell& other) const noexcept {
        return !(*this == other);
    }
};

class ScreenBuffer {
public:
    ScreenBuffer() = default;
    ScreenBuffer(int width, int height, Cell fill_cell = Cell{});

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    int size() const noexcept { return width_ * height_; }

    bool in_bounds(int x, int y) const noexcept {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    const Cell& get(int x, int y) const;
    Cell& get(int x, int y);

    bool set(int x, int y, const Cell& cell);
    bool set(int x, int y, char ch, Color fg = Color::Default, Color bg = Color::Default);

    void clear(const Cell& clear_cell = Cell{});
    void fill(int x, int y, int w, int h, const Cell& cell);
    void write_string(int x, int y, std::string_view text, Color fg = Color::Default, Color bg = Color::Default);

    void resize(int width, int height, Cell fill_cell = Cell{});
    const std::vector<Cell>& cells() const noexcept { return cells_; }

private:
    int width_{0};
    int height_{0};
    std::vector<Cell> cells_;
};

struct CellDiff {
    int x{0};
    int y{0};
    Cell cell{};

    bool operator==(const CellDiff& other) const noexcept {
        return x == other.x && y == other.y && cell == other.cell;
    }
};

struct BufferDiff {
    int width{0};
    int height{0};
    std::vector<CellDiff> changes;
    bool is_full_redraw{false};
};

// Computes changes between front (current) and back (previous) buffer
BufferDiff compute_diff(const ScreenBuffer& current, const ScreenBuffer& previous, bool force_full = false);

// Converts BufferDiff to ANSI escape sequence string
std::string render_diff_to_ansi(const BufferDiff& diff);

} // namespace asciiforge

