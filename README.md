# C + Raylib Hot Reload Template
This is a C + Raylib game template with a Hot Reloading workflow. It makes it possible to make changes to C code and reload it on the fly while the game is running, allowing for very small iteration times.

Once the application is built and running, you don't need to run build commands anymore, hot reload kicks in automatically as soon as you edit and save any source file.

Supported platforms: macOS (tested), Windows (tested), Linux (untested).

![c-raylib-hot-reload-sample](https://github.com/user-attachments/assets/8b15bac3-59cf-4e3b-bb4d-9a5b43bfbb3e)

## How to run the Hot Reload workflow
- Open the project, run `./build_hot_reload.sh run` (or `./build_hot_reload.bat run`).
- In VSCode, instead of manually running the build scripts, with `F5`, you can run the tasks `Hot Reload: Build and Run (Linux / Mac)` (or Windows).
- Make changes to `game.c` (or any other related file, you can also add and remove files, as the hot reload workflow account for added and remove files).
- Watch as the game reloads as soon as you hit save.
- To force rebuild, while the game is running, you can call `./build_hot_reload.sh` (without `run`).
- In the sample project, you can reload the game library with `F5` or restart it (reset state) with `F6`.

### How it Works
- The host hot reload application is in `main_hot_reload.c`.
- The game/application code goes in `game.c`.
- For debug and release builds, `main.c` is used instead.
- In the hot reload workflow, the game is built as a shared library. In the debug and release builds, a standalone application is built.
- To watch for file changes, additions and removals in real-time, [file_version_builder.c](src/hot_reload/file_version_builder.c) keeps a list of source files and their modification times (into a dev temp file `file_versions.dat`) and [file_versions.h](src/hot_reload/file_versions.h) watches that list.

## Extra Features
- For Debug and Release the project can be built with either make or CMake, as it contains essential Makefile and CMake files.
- raylib added as a dependency in a subfolder, so it can be changed, debugged and introspected with the project.
- Contains VSCode configuration.
- Direct control over compiling both raylib and the project to release.

## Getting Started
Clone with submodules:
```
git clone --recurse-submodules https://github.com/alfredbaudisch/raylib_hot_reload_template
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

## A note about the Raylib generated API and Raylib version
- [raylib_api.gen.h](src/hot_reload/raylib_api.gen.h) is used by the hot reload workflow. `raylib_api.gen.h` is transparent during development, as long as `HOT_RELOAD` is not defined in the editor/IDE, it's `raylib.h` that will normally show up in auto completions and the like. 
- A standalone/normal build (for debug or release) uses `raylib.h` directly.
- If you change to a different version of raylib either by a major release or another commit hash in the submodule, you have to rebuild `raylib_api.gen.h`, for that you need Python3 then simply run `make generate_raylib_api` or `python3 generate_raylib_api.py`.

## Acknowledgements
This is heavily inspired and adapted from [Odin + Raylib + Hot Reload template](https://github.com/karl-zylinski/odin-raylib-hot-reload-game-template) by Karl Zylinski (where I also did some small contributions).
