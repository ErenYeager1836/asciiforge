#include <catch2/catch_test_macros.hpp>
#include "asciiforge/buffer.hpp"

using namespace asciiforge;

TEST_CASE("Diff output: no change produces empty diff", "[renderer]") {
    ScreenBuffer front(5, 5, Cell{'.', Color::White, Color::Black});
    ScreenBuffer back(5, 5, Cell{'.', Color::White, Color::Black});

    BufferDiff diff = compute_diff(front, back);

    CHECK(diff.changes.empty());
    CHECK_FALSE(diff.is_full_redraw);
    CHECK(render_diff_to_ansi(diff).empty());
}

TEST_CASE("Diff output: single cell change produces exactly one change", "[renderer]") {
    ScreenBuffer front(4, 4, Cell{'.', Color::White, Color::Black});
    ScreenBuffer back(4, 4, Cell{'.', Color::White, Color::Black});

    front.set(2, 1, 'X', Color::BrightYellow, Color::Black);

    BufferDiff diff = compute_diff(front, back);

    REQUIRE(diff.changes.size() == 1);
    CHECK(diff.changes[0].x == 2);
    CHECK(diff.changes[0].y == 1);
    CHECK(diff.changes[0].cell.ch == 'X');
    CHECK(diff.changes[0].cell.fg == Color::BrightYellow);

    std::string ansi = render_diff_to_ansi(diff);
    // Row 1 is 1-based index 2, Col 2 is 1-based index 3 -> \033[2;3H
    CHECK(ansi.find("\033[2;3H") != std::string::npos);
    CHECK(ansi.find('X') != std::string::npos);
}

TEST_CASE("Diff output: color-only change produces diff with same character", "[renderer]") {
    ScreenBuffer front(3, 3, Cell{'#', Color::BrightRed, Color::Black});
    ScreenBuffer back(3, 3, Cell{'#', Color::BrightBlue, Color::Black});

    // Only change color at (1, 1)
    back.set(0, 0, '#', Color::BrightRed, Color::Black);
    back.set(0, 1, '#', Color::BrightRed, Color::Black);
    back.set(0, 2, '#', Color::BrightRed, Color::Black);
    back.set(1, 0, '#', Color::BrightRed, Color::Black);
    back.set(1, 2, '#', Color::BrightRed, Color::Black);
    back.set(2, 0, '#', Color::BrightRed, Color::Black);
    back.set(2, 1, '#', Color::BrightRed, Color::Black);
    back.set(2, 2, '#', Color::BrightRed, Color::Black);

    BufferDiff diff = compute_diff(front, back);

    REQUIRE(diff.changes.size() == 1);
    CHECK(diff.changes[0].x == 1);
    CHECK(diff.changes[0].y == 1);
    CHECK(diff.changes[0].cell.ch == '#');
    CHECK(diff.changes[0].cell.fg == Color::BrightRed);

    std::string ansi = render_diff_to_ansi(diff);
    // BrightRed ANSI code is 91
    CHECK(ansi.find("\033[91m") != std::string::npos);
}

TEST_CASE("Diff output: full redraw emits all cells", "[renderer]") {
    ScreenBuffer front(3, 2, Cell{'A', Color::White, Color::Black});
    ScreenBuffer back(3, 2, Cell{'A', Color::White, Color::Black});

    BufferDiff diff = compute_diff(front, back, /*force_full=*/true);

    CHECK(diff.is_full_redraw);
    CHECK(diff.changes.size() == 6);
}

TEST_CASE("Diff output: buffer resize forces full redraw", "[renderer]") {
    ScreenBuffer front(4, 4, Cell{'A', Color::White, Color::Black});
    ScreenBuffer back(3, 3, Cell{'A', Color::White, Color::Black});

    BufferDiff diff = compute_diff(front, back);

    CHECK(diff.is_full_redraw);
    CHECK(diff.changes.size() == 16);
}

TEST_CASE("Diff rendering: adjacent cells on same row omit redundant cursor movements", "[renderer]") {
    ScreenBuffer front(5, 1, Cell{' ', Color::Default, Color::Default});
    ScreenBuffer back(5, 1, Cell{' ', Color::Default, Color::Default});

    front.set(1, 0, 'A', Color::White, Color::Black);
    front.set(2, 0, 'B', Color::White, Color::Black);
    front.set(3, 0, 'C', Color::White, Color::Black);

    BufferDiff diff = compute_diff(front, back);
    REQUIRE(diff.changes.size() == 3);

    std::string ansi = render_diff_to_ansi(diff);

    // Initial position at col 2 (1-based), row 1: \033[1;2H
    CHECK(ansi.find("\033[1;2H") != std::string::npos);

    // Should NOT contain reposition for col 3 or 4: \033[1;3H or \033[1;4H
    CHECK(ansi.find("\033[1;3H") == std::string::npos);
    CHECK(ansi.find("\033[1;4H") == std::string::npos);
    CHECK(ansi.find("ABC") != std::string::npos);
}

