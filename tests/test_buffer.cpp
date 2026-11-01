#include <catch2/catch_test_macros.hpp>
#include "asciiforge/buffer.hpp"

using namespace asciiforge;

TEST_CASE("ScreenBuffer dimensions and boundary checks", "[buffer]") {
    ScreenBuffer buffer(10, 5);

    REQUIRE(buffer.width() == 10);
    REQUIRE(buffer.height() == 5);
    REQUIRE(buffer.size() == 50);

    CHECK(buffer.in_bounds(0, 0));
    CHECK(buffer.in_bounds(9, 4));
    CHECK_FALSE(buffer.in_bounds(-1, 0));
    CHECK_FALSE(buffer.in_bounds(0, -1));
    CHECK_FALSE(buffer.in_bounds(10, 0));
    CHECK_FALSE(buffer.in_bounds(0, 5));
}

TEST_CASE("ScreenBuffer set and get valid cell", "[buffer]") {
    ScreenBuffer buffer(4, 4);

    Cell test_cell{'X', Color::BrightYellow, Color::Blue};
    CHECK(buffer.set(2, 3, test_cell));

    const Cell& retrieved = buffer.get(2, 3);
    CHECK(retrieved.ch == 'X');
    CHECK(retrieved.fg == Color::BrightYellow);
    CHECK(retrieved.bg == Color::Blue);
}

TEST_CASE("ScreenBuffer out of bounds get throws exception", "[buffer]") {
    ScreenBuffer buffer(3, 3);

    CHECK_THROWS_AS(buffer.get(-1, 0), std::out_of_range);
    CHECK_THROWS_AS(buffer.get(3, 2), std::out_of_range);
    CHECK_THROWS_AS(buffer.get(0, 3), std::out_of_range);
}

TEST_CASE("ScreenBuffer out of bounds set returns false safely", "[buffer]") {
    ScreenBuffer buffer(3, 3);

    CHECK_FALSE(buffer.set(-1, 0, Cell{'A', Color::White, Color::Black}));
    CHECK_FALSE(buffer.set(3, 1, Cell{'B', Color::White, Color::Black}));
    CHECK_FALSE(buffer.set(1, 3, Cell{'C', Color::White, Color::Black}));
}

TEST_CASE("ScreenBuffer clear with default and custom cell", "[buffer]") {
    ScreenBuffer buffer(3, 2);
    buffer.set(1, 1, 'Z', Color::BrightRed);

    buffer.clear(Cell{'.', Color::Green, Color::Default});

    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 3; ++x) {
            CHECK(buffer.get(x, y).ch == '.');
            CHECK(buffer.get(x, y).fg == Color::Green);
            CHECK(buffer.get(x, y).bg == Color::Default);
        }
    }
}

TEST_CASE("ScreenBuffer fill rectangular region", "[buffer]") {
    ScreenBuffer buffer(5, 5, Cell{' ', Color::Default, Color::Default});

    buffer.fill(1, 1, 3, 2, Cell{'#', Color::Cyan, Color::Default});

    // Filled region
    for (int y = 1; y < 3; ++y) {
        for (int x = 1; x < 4; ++x) {
            CHECK(buffer.get(x, y).ch == '#');
            CHECK(buffer.get(x, y).fg == Color::Cyan);
        }
    }

    // Unfilled borders
    CHECK(buffer.get(0, 0).ch == ' ');
    CHECK(buffer.get(4, 4).ch == ' ');
}

TEST_CASE("ScreenBuffer fill with clipping outside bounds", "[buffer]") {
    ScreenBuffer buffer(4, 4, Cell{'.', Color::Default, Color::Default});

    // Fill partially off the left/top edge
    buffer.fill(-2, -2, 4, 4, Cell{'X', Color::Red, Color::Default});

    CHECK(buffer.get(0, 0).ch == 'X');
    CHECK(buffer.get(1, 1).ch == 'X');
    CHECK(buffer.get(2, 2).ch == '.');

    // Fill partially off bottom/right edge
    buffer.fill(2, 2, 5, 5, Cell{'Y', Color::Blue, Color::Default});
    CHECK(buffer.get(2, 2).ch == 'Y');
    CHECK(buffer.get(3, 3).ch == 'Y');
}

TEST_CASE("ScreenBuffer write_string handles valid and boundary clipping", "[buffer]") {
    ScreenBuffer buffer(6, 2, Cell{'.', Color::Default, Color::Default});

    buffer.write_string(1, 0, "HI", Color::BrightWhite);
    CHECK(buffer.get(0, 0).ch == '.');
    CHECK(buffer.get(1, 0).ch == 'H');
    CHECK(buffer.get(2, 0).ch == 'I');
    CHECK(buffer.get(3, 0).ch == '.');

    // String that exceeds right border should be safely clipped
    buffer.write_string(4, 1, "OVERFLOW", Color::Yellow);
    CHECK(buffer.get(4, 1).ch == 'O');
    CHECK(buffer.get(5, 1).ch == 'V');
}

TEST_CASE("ScreenBuffer resize preserves existing data", "[buffer]") {
    ScreenBuffer buffer(2, 2, Cell{'A', Color::Default, Color::Default});
    buffer.resize(3, 3, Cell{'B', Color::Default, Color::Default});

    REQUIRE(buffer.width() == 3);
    REQUIRE(buffer.height() == 3);

    // Old data preserved
    CHECK(buffer.get(0, 0).ch == 'A');
    CHECK(buffer.get(1, 1).ch == 'A');

    // New area filled with fill_cell
    CHECK(buffer.get(2, 0).ch == 'B');
    CHECK(buffer.get(2, 2).ch == 'B');
}

