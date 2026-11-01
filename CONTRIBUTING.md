# Contributing to asciiforge

Thank you for your interest in contributing to asciiforge!

## Standards and Guidelines
- **Language**: Modern C++17.
- **Build System**: CMake >= 3.20.
- **Dependencies**: No external runtime dependencies. Unit testing is performed via Catch2 (fetched automatically via FetchContent).
- **Warnings**: All code must compile cleanly with zero warnings under `/W4 /WX` on MSVC and `-Wall -Wextra -Wpedantic -Werror` on GCC/Clang.
- **Platform Separation**: Core game logic (buffers, diff rendering, game loop, map loading, collision, entities) must remain platform-independent. Platform-specific console I/O must reside only under `src/platform/`.

## Local Build & Test
### Windows
```bash
cmake -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

### Ubuntu / Linux
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Submitting Pull Requests
- Keep changes focused and logical.
- Add unit tests for new features and edge cases.
- Verify all unit tests pass before submitting.
