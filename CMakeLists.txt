cmake_minimum_required(VERSION 3.20)
project(asciiforge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warning flags: zero warnings tolerated
if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Core library (platform-independent)
add_library(asciiforge_core STATIC
    src/buffer.cpp
    src/game_loop.cpp
    src/map.cpp
    src/entity.cpp
    src/collision.cpp
)

target_include_directories(asciiforge_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Platform library
set(PLATFORM_SOURCES
    src/platform/terminal_null.cpp
)

if(WIN32)
    list(APPEND PLATFORM_SOURCES src/platform/terminal_win32.cpp)
endif()

add_library(asciiforge_platform STATIC
    ${PLATFORM_SOURCES}
)

target_include_directories(asciiforge_platform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(asciiforge_platform
    PUBLIC
        asciiforge_core
)

# Demo executable
add_executable(asciiforge_demo
    demo/main.cpp
)

target_link_libraries(asciiforge_demo
    PRIVATE
        asciiforge_core
        asciiforge_platform
)

# Copy demo map assets to output directory
add_custom_command(
    TARGET asciiforge_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/demo/maps"
            "$<TARGET_FILE_DIR:asciiforge_demo>/maps"
)

# Tests with Catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(asciiforge_tests
    tests/test_buffer.cpp
    tests/test_diff_renderer.cpp
    tests/test_map.cpp
    tests/test_collision.cpp
    tests/test_game_loop.cpp
    tests/test_entity.cpp
    tests/test_terminal_null.cpp
)

target_link_libraries(asciiforge_tests
    PRIVATE
        asciiforge_core
        asciiforge_platform
        Catch2::Catch2WithMain
)

include(CTest)
add_test(NAME asciiforge_unit_tests COMMAND asciiforge_tests)

