Template to make it easy to code raylib projects with C:
- Can be built with either make or CMake, as it contains essential Makefile and CMake files.
- raylib added as a dependency in a subfolder, so it can be changed, debugged and introspected with the project.
- Contains VSCode configuration.
- Direct control over compiling both raylib and the project to release.

## Getting Started
Clone with submodules:
```
git clone --recurse-submodules https://github.com/alfredbaudisch/EasyRaylib
```

To update:
```
git submodule update --recursive
```

## Building
It's possible to build with `make` or `CMake`.

### Make
- Dev/Debug: `make`
- Release: `make MODE=RELEASE`
- Only the game (to avoid recompiling raylib everytime): `make game` or `make game MODE=RELEASE`

### CMake
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

Or Release (case-sensitive): `-DCMAKE_BUILD_TYPE=Release`.

In case CMake uses Visual Studio by default and compilation is failing, force CMake to use MinGW. Two ways:
- Set the env variable `CMAKE_GENERATOR="MinGW Makefiles"`.
- Call with `cmake -G "MinGW Makefiles" ..`. Example: `cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..`.
  - Notice that it has to be called with `-G "MinGW Makefiles"` only once, when generating the project. Calls to `--build` won't require the parameter.

### Requirements
#### Windows
- MSYS2 with GCC tooling: https://www.msys2.org/
  - After installing it, run in the MSYS2 MINGW64 terminal: `pacman -S mingw-w64-ucrt-x86_64-gcc`
  - Add the MSYS2 and mingw64 paths to PATH, example: `C:\Dev\msys2\usr\bin` and `C:\Dev\msys2\mingw64\bin`.
- CMake
