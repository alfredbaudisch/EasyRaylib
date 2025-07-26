# C + Raylib Hot Reload Template
This is a C + Raylib game template with a Hot Reloading workflow. It makes it possible to make changes to C code and reload it on the fly while the game is running, allowing for very small iteration times.

Once the application is built and running, you don't need to run build commands anymore, hot reload kicks in automatically as soon as you edit and save any source file.

Supported platforms: macOS, Linux and Windows (untested).

## WARNING about Windows ⚠️
Currently I only have a Macbook, and I managed to build on a Windows VM, but I wasn't able to run it due to the lack of OpenGL support on Parallels. So I'm looking for contributors to make sure the Windows setup works.

## How to run the Hot Reload workflow
- Open the project, run `./build_hot_reload.sh run` (or `./build_hot_reload.bat run`).
- Make changes to `game.c` (or any other related file, you can also add and remove files, as the hot reload workflow account for added and remove files).
- Watch as the game reloads as soon as you hit save.
- To force rebuild, while the game is running, you can call `./build_hot_reload.sh` (without `run`).
- In the sample project, you can reload the game library with `F5` or restart it (reset state) with `F6`.
- In VSCode, instead of manually running the build scripts, with `F5`, you can run the tasks `Hot Reload: Build and Run (Linux / Mac)` (or Windows).

### How it Works
- The host hot reload application is in `main_hot_reload.c`.
- The game/application code goes in `game.c`.
- For debug and release builds, `main.c` is used instead.

## Extra Features
- For Debug and Release the project can be built with either make or CMake, as it contains essential Makefile and CMake files.
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

## Building for Debug and Release
To build for Debug or Release, `main.c` is used instead of `main_hot_reload.c`. It's possible to build with `make` or `CMake`.

### Make
- Debug: `make`
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
#### MacOS
The same requirements as raylib.

#### Windows
- MSYS2 with GCC tooling: https://www.msys2.org/
  - After installing it, run in the MSYS2 MINGW64 terminal: `pacman -S mingw-w64-ucrt-x86_64-gcc`
  - Add the MSYS2 and mingw64 paths to PATH, example: `C:\Dev\msys2\usr\bin` and `C:\Dev\msys2\mingw64\bin`.
- CMake

## Acknowledgements
This is heavily inspired and adapted from [Odin + Raylib + Hot Reload template](https://github.com/karl-zylinski/odin-raylib-hot-reload-game-template) by Karl Zylinski (where I also did some small contributions).