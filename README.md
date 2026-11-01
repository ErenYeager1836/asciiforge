# asciiforge

asciiforge is a small C++17 ASCII terminal game engine skeleton. It provides a double-buffered diff renderer, a fixed-timestep game loop with sub-step interpolation, a text-based tile map loader, grid collision, basic entities (player and patrolling enemies), and platform console abstraction.

<!-- DEMO GIF PLACEHOLDER: demo/media/asciiforge_demo.gif -->
> **[Demo Recording Placeholder]**
> *(GIF demonstration of asciiforge terminal gameplay will be placed here)*

## Status

**v0.1 (In Progress - Round 1 of 3)**
- Core platform-independent library (buffer, diff rendering, game loop, tile map, entity, collision).
- Windows console backend using Win32 API and ANSI virtual terminal processing.
- Null terminal backend for headless execution and deterministic unit testing in CI.
- Room 1 playable demo featuring player movement, enemy patrols, collisions, and HUD stats.

## Project Layout

```text
asciiforge/
  include/asciiforge/   Public headers (buffer, clock, collision, entity, game_loop, map, terminal)
  src/                  Core engine implementation:
                          buffer.cpp, game_loop.cpp, map.cpp, entity.cpp, collision.cpp
  src/platform/         Console platform implementations:
                          terminal.hpp (interface), terminal_win32.cpp, terminal_null.cpp
  demo/                 Playable demonstration (main.cpp and maps/room1.txt)
  tests/                Catch2 unit tests covering all engine modules
  CMakeLists.txt        Build configuration (C++17, CMake >= 3.20)
  README.md             Project documentation
  CONTRIBUTING.md       Contribution standards
  LICENSE               MIT License
  .gitignore            Ignored build and IDE artifacts
  .github/workflows/    GitHub Actions CI matrix (Windows MSVC + Ubuntu GCC)
```

## Building

### Requirements
- C++17 compliant compiler (MSVC 2019+ on Windows, GCC 9+ or Clang 10+ on Linux)
- CMake >= 3.20
- Git (used by CMake FetchContent to download Catch2 for unit tests)

### Windows (MSVC)
```powershell
cmake -B build
cmake --build build --config Release
```

### Ubuntu / Linux (GCC)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Running the Demo

On Windows:
```powershell
.\build\Release\asciiforge_demo.exe
```

On Linux:
```bash
./build/asciiforge_demo
```

### Controls
| Key | Action |
| --- | --- |
| `W`, `A`, `S`, `D` or `Arrow Keys` | Move Player (`@`) |
| `Q` or `Esc` | Quit Demo |

## Running Tests

Unit tests are built using Catch2 and can be executed via CTest or directly:

```powershell
ctest --test-dir build -C Release --output-on-failure
```

Direct binary execution:
```powershell
.\build\Release\asciiforge_tests.exe
```

## Roadmap

- Linux terminal input (planned, not started)
- Colors/UI widgets (planned, not started)
- Networking (planned, not started)

## License

MIT License. See [LICENSE](LICENSE) for details.
