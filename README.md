# C + Raylib Hot Reload Template
This is a C + Raylib game template with a Hot Reloading workflow. It makes it possible to make changes to C code and reload it on the fly while the game is running, allowing for very small iteration times, preserving runtime state. It's also possible to debug the hot reloaded code (see instructions below).

It also comes with an optional File Watcher (enabled by default): once the application is built and running, you don't need to run build commands anymore, hot reload kicks in automatically as soon as you edit and save any source file.

Supported platforms: macOS (tested), Windows (tested), Linux (untested).

![c-raylib-hot-reload-sample](https://github.com/user-attachments/assets/8b15bac3-59cf-4e3b-bb4d-9a5b43bfbb3e)

## How to run the Hot Reload workflow
- Open the project, run `./build_hot_reload.sh run` (or `./build_hot_reload.bat run`).
- In VSCode, instead of manually running the build scripts, with `F5`, you can run the tasks `Hot Reload: Build and Run (Linux / Mac)` (or Windows).
- Make changes to `game.c` (or any other related file, you can also add and remove files, as the hot reload workflow account for added and removed files).
- To force rebuild, while the game is running, you can call `./build_hot_reload.sh` (without `run`).
- In the sample project, you can reload the game library with `F5` or restart it (reset state) with `F6`.

### File Watcher
- If the File Watcher is active (`-DHOT_RELOAD_FILE_WATCHER` in the build script), the game rebuilds and reloads as soon as you hit save in `game.c` and any other related file.
- To disable automatic rebuilds with the file watcher, undef `HOT_RELOAD_FILE_WATCHER` (i.e. remove `-DHOT_RELOAD_FILE_WATCHER` from `build_hot_reload.bat` or `build_hot_reload.sh`).
  - With the file watcher disabled, run the hot reload workflow with `./build_hot_reload.sh run` (or `./build_hot_reload.bat run`), and then anytime you want to hot reload again, run just `./build_hot_reload.sh` (or `./build_hot_reload.bat`), without `run`.
  - You can also call VSCode's build task `Hot Reload: Build`.

### How it Works
- The host hot reload application is in `main_hot_reload.c`.
- The game/application code goes in `game.c`.
- For debug and release builds, `main.c` is used instead.
- In the hot reload workflow, the game is built as a shared library. In the debug and release builds, a standalone application is built.
- If the file watcher is active (it's on by default), to watch for file changes, additions and removals in real-time, [file_version_builder.c](src/hot_reload/file_version_builder.c) builds a list of source files and their modification times into a dev temp file `file_versions.dat` and [file_watcher.h](src/hot_reload/file_watcher.h) watches that list.

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

## Requirements
#### MacOS
The same requirements as raylib.

#### Windows
- MSYS2 with GCC tooling: https://www.msys2.org/
  - After installing it, run in the MSYS2 MINGW64 terminal: `pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain`
  - Add the MSYS2 and mingw64 paths to PATH, example: `C:\msys64\ucrt64\bin`.
  - For more, see https://code.visualstudio.com/docs/cpp/config-mingw

## Debugging
### Hot Reload Debug (Mac/Linux only for now)
- Run `./build_hot_reload_debug.sh`.
- You can use debugging tools as normal.
- To hot reload, call `./build_hot_reload_debug.sh` anytime again, even while the debugger is running.
- You can also debug on VSCode, for that install the [CodeLLDB extension](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) and then run `Hot Reload: Debug`.

### Regular Build Debug (Win, Mac, Linux)
For the regular build (without hot reload, which uses `main.c`):
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

- You can also debug on VSCode, for that install the [CodeLLDB extension](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) and then run `Debug Regular Build`.

## Building for Release (WITHOUT hot reload)
It's possible to build for Release with `make` or `CMake`.

### Make
```
make MODE=RELEASE
```

- Only the game (to avoid recompiling raylib everytime): `make game` or `make game MODE=RELEASE`.
- You can also run `Release (Make)` on VSCode.

### CMake
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

- You can also run `Release (CMake)` on VSCode.
- In case CMake uses Visual Studio by default and compilation is failing, force CMake to use MinGW. Two ways:
  - Set the env variable `CMAKE_GENERATOR="MinGW Makefiles"`.
  - Call with `cmake -G "MinGW Makefiles" ..`. Example: `cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..`.
    - Notice that it has to be called with `-G "MinGW Makefiles"` only once, when generating the project. Calls to `--build` won't require the parameter.

## A note about the Raylib generated API and Raylib version
- [raylib_api.gen.h](src/hot_reload/raylib_api.gen.h) is used by the hot reload workflow. `raylib_api.gen.h` is transparent during development, as long as `HOT_RELOAD` is not defined in the editor/IDE, it's `raylib.h` that will normally show up in auto completions and the like. 
- A standalone/normal build (for debug or release) uses `raylib.h` directly.
- If you change to a different version of raylib either by a major release or another commit hash in the submodule, you have to rebuild `raylib_api.gen.h`, for that you need Python3 then simply run `make generate_raylib_api` or `python3 generate_raylib_api.py`.

## Acknowledgements
This is heavily inspired and adapted from [Odin + Raylib + Hot Reload template](https://github.com/karl-zylinski/odin-raylib-hot-reload-game-template) by Karl Zylinski (where I also did some small contributions).
