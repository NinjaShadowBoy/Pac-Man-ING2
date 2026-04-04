# Pac Man ING2

A Pac-Man style game with randomly generated mazes, built with SDL2.

## Canonical build system

The canonical/reproducible build entry point is now **CMake** (`CMakeLists.txt` at repo root).

`Projectp.dev` is kept as an optional legacy IDE project file for Dev-C++ users, but it is **not** the source of truth for CI/release builds.

## Dependencies

You need the following development packages:

- SDL2
- SDL2_image
- SDL2_mixer
- SDL2_ttf
- C compiler (GCC/Clang/MSVC)
- CMake 3.16+

## Build on Linux (Ubuntu/Debian example)

Install dependencies:

```bash
sudo apt update
sudo apt install -y build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
```

Configure and build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run:

```bash
./build/Projectp
```

## Build on Windows (MSYS2/MinGW recommended)

Install dependencies in **MSYS2 MinGW x64 shell**:

```bash
pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_ttf
```

Configure and build:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run:

```bash
./build/Projectp.exe
```

## Legacy Dev-C++/MinGW makefile (optional)

`Makefile.win` is kept for compatibility and now supports configurable paths.

Examples:

```bash
mingw32-make -f Makefile.win
```

With explicit tool/dependency locations:

```bash
mingw32-make -f Makefile.win MINGW_PREFIX="C:/Program Files (x86)/Dev-Cpp/MinGW64" SDL2_DIR="C:/SDL2/x86_64-w64-mingw32"
```
