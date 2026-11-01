#include "asciiforge/buffer.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace asciiforge {

int ansi_fg_code(Color color) noexcept {
    const auto val = static_cast<uint8_t>(color);
    if (val <= 7) {
        return 30 + val;
    }
    if (val <= 15) {
        return 90 + (val - 8);
    }
    return 39; // Default foreground
}

int ansi_bg_code(Color color) noexcept {
    const auto val = static_cast<uint8_t>(color);
    if (val <= 7) {
        return 40 + val;
    }
    if (val <= 15) {
        return 100 + (val - 8);
    }
    return 49; // Default background
}

ScreenBuffer::ScreenBuffer(int width, int height, Cell fill_cell)
    : width_(std::max(0, width)),
      height_(std::max(0, height)),
      cells_(static_cast<size_t>(width_ * height_), fill_cell) {}

const Cell& ScreenBuffer::get(int x, int y) const {
    if (!in_bounds(x, y)) {
        throw std::out_of_range("ScreenBuffer::get: coordinates out of range");
    }
    return cells_[static_cast<size_t>(y * width_ + x)];
}

Cell& ScreenBuffer::get(int x, int y) {
    if (!in_bounds(x, y)) {
        throw std::out_of_range("ScreenBuffer::get: coordinates out of range");
    }
    return cells_[static_cast<size_t>(y * width_ + x)];
}

bool ScreenBuffer::set(int x, int y, const Cell& cell) {
    if (!in_bounds(x, y)) {
        return false;
    }
    cells_[static_cast<size_t>(y * width_ + x)] = cell;
    return true;
}

bool ScreenBuffer::set(int x, int y, char ch, Color fg, Color bg) {
    return set(x, y, Cell{ch, fg, bg});
}

void ScreenBuffer::clear(const Cell& clear_cell) {
    std::fill(cells_.begin(), cells_.end(), clear_cell);
}

void ScreenBuffer::fill(int x, int y, int w, int h, const Cell& cell) {
    if (w <= 0 || h <= 0 || width_ <= 0 || height_ <= 0) {
        return;
    }
    const int x_start = std::max(0, x);
    const int y_start = std::max(0, y);
    const int x_end = std::min(width_, x + w);
    const int y_end = std::min(height_, y + h);

    for (int cy = y_start; cy < y_end; ++cy) {
        for (int cx = x_start; cx < x_end; ++cx) {
            cells_[static_cast<size_t>(cy * width_ + cx)] = cell;
        }
    }
}

void ScreenBuffer::write_string(int x, int y, std::string_view text, Color fg, Color bg) {
    if (y < 0 || y >= height_) {
        return;
    }
    for (size_t i = 0; i < text.size(); ++i) {
        const int cx = x + static_cast<int>(i);
        if (cx >= width_) {
            break;
        }
        if (cx >= 0) {
            set(cx, y, Cell{text[i], fg, bg});
        }
    }
}

void ScreenBuffer::resize(int width, int height, Cell fill_cell) {
    const int new_w = std::max(0, width);
    const int new_h = std::max(0, height);
    if (new_w == width_ && new_h == height_) {
        return;
    }

    std::vector<Cell> new_cells(static_cast<size_t>(new_w * new_h), fill_cell);
    const int copy_w = std::min(width_, new_w);
    const int copy_h = std::min(height_, new_h);

    for (int y = 0; y < copy_h; ++y) {
        for (int x = 0; x < copy_w; ++x) {
            new_cells[static_cast<size_t>(y * new_w + x)] = cells_[static_cast<size_t>(y * width_ + x)];
        }
    }

    width_ = new_w;
    height_ = new_h;
    cells_ = std::move(new_cells);
}

BufferDiff compute_diff(const ScreenBuffer& current, const ScreenBuffer& previous, bool force_full) {
    BufferDiff diff;
    diff.width = current.width();
    diff.height = current.height();

    const bool dimensions_changed = (current.width() != previous.width() || current.height() != previous.height());
    diff.is_full_redraw = force_full || dimensions_changed;

    if (diff.is_full_redraw) {
        diff.changes.reserve(static_cast<size_t>(current.size()));
        for (int y = 0; y < current.height(); ++y) {
            for (int x = 0; x < current.width(); ++x) {
                diff.changes.push_back(CellDiff{x, y, current.get(x, y)});
            }
        }
    } else {
        for (int y = 0; y < current.height(); ++y) {
            for (int x = 0; x < current.width(); ++x) {
                const Cell& curr = current.get(x, y);
                const Cell& prev = previous.get(x, y);
                if (curr != prev) {
                    diff.changes.push_back(CellDiff{x, y, curr});
                }
            }
        }
    }

    return diff;
}

std::string render_diff_to_ansi(const BufferDiff& diff) {
    if (diff.changes.empty()) {
        return "";
    }

    std::ostringstream out;
    int last_x = -999;
    int last_y = -999;
    Color current_fg = Color::Default;
    Color current_bg = Color::Default;
    bool has_active_style = false;

    for (const auto& change : diff.changes) {
        // Move cursor if not immediately adjacent to previous char on same row
        if (change.y != last_y || change.x != last_x + 1) {
            // ANSI escape for cursor position is 1-based (row, col)
            out << "\033[" << (change.y + 1) << ";" << (change.x + 1) << "H";
        }

        // Foreground color check
        if (!has_active_style || change.cell.fg != current_fg) {
            out << "\033[" << ansi_fg_code(change.cell.fg) << "m";
            current_fg = change.cell.fg;
        }

        // Background color check
        if (!has_active_style || change.cell.bg != current_bg) {
            out << "\033[" << ansi_bg_code(change.cell.bg) << "m";
            current_bg = change.cell.bg;
        }

        has_active_style = true;
        out << change.cell.ch;

        last_x = change.x;
        last_y = change.y;
    }

    // Reset ANSI styling at the end to keep terminal state tidy
    out << "\033[0m";
    return out.str();
}

} // namespace asciiforge

