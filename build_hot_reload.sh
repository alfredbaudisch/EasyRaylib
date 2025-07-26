#!/bin/bash -eu

# OUT_DIR is for everything except the exe. The exe needs to stay in root
# folder so it sees the resources folder, without having to copy it.
OUT_DIR=build/hot_reload
EXE=game_hot_reload

mkdir -p $OUT_DIR

# Build raylib first if it doesn't exist
if [ ! -f "deps/raylib/src/libraylib.a" ]; then
    echo "Building raylib..."
    cd deps/raylib/src && make PLATFORM=PLATFORM_DESKTOP
    cd ../../..
fi

# Figure out which DLL extension to use based on platform
case $(uname) in
"Darwin")
    DLL_EXT=".dylib"
    SHARED_FLAGS="-dynamiclib"
    CC=clang
    ;;
"Linux")
    DLL_EXT=".so"
    SHARED_FLAGS="-shared -fPIC"
    CC=gcc
    ;;
*)
    DLL_EXT=".so"
    SHARED_FLAGS="-shared -fPIC"  
    CC=gcc
    ;;
esac

# Build the game DLL with weak symbols (will resolve from main executable)
echo "Building game$DLL_EXT"

# Find all C files except main.c and main_hot_reload.c for the game library
GAME_SOURCES=$(find src -name "*.c" -not -name "main.c" -not -name "main_hot_reload.c" -not -name "file_version_builder.c" -not -name "platform_tools.c")

# Building file version builder
$CC -g -O0 -std=c99 \
    src/hot_reload/file_version_builder.c \
    src/hot_reload/platform_tools.c \
    -o $OUT_DIR/file_version_builder
./$OUT_DIR/file_version_builder

# Compile game as shared library
$CC $SHARED_FLAGS -g -O0 -std=c99 \
    -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -DHOT_RELOAD \
    -Ideps/raylib/src -Isrc \
    $GAME_SOURCES \
    -o $OUT_DIR/game_tmp$DLL_EXT

# Need to use a temp file because the loader might try to load the DLL
# before it's fully written
mv $OUT_DIR/game_tmp$DLL_EXT $OUT_DIR/game$DLL_EXT

# If the executable is already running, then don't try to build and start it.
if pgrep -f $EXE > /dev/null; then
    echo "Hot reloading..."
    exit 0
fi

echo "Building $EXE"
# Link raylib ONLY to the main executable
case $(uname) in
"Darwin")
    RAYLIB_LIBS="deps/raylib/src/libraylib.a -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreAudio"
    ;;
"Linux")
    RAYLIB_LIBS="deps/raylib/src/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11"
    ;;
*)
    RAYLIB_LIBS="deps/raylib/src/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11"
    ;;
esac

# Set export flags for main executable
case $(uname) in
"Darwin")
    EXPORT_FLAGS="-rdynamic"
    ;;
"Linux")
    EXPORT_FLAGS="-Wl,-export-dynamic"
    ;;
*)
    EXPORT_FLAGS="-Wl,-export-dynamic"
    ;;
esac

$CC -g -O0 -std=c99 \
    -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 \
    -Ideps/raylib/src -Isrc \
    src/main_hot_reload.c \
    src/hot_reload/platform_tools.c \
    $RAYLIB_LIBS \
    $EXPORT_FLAGS \
    -o $OUT_DIR/$EXE -ldl

echo "Build complete!"
echo "  Main executable: $OUT_DIR/$EXE"
echo "  Game library: $OUT_DIR/game$DLL_EXT"
echo ""

if [ $# -ge 1 ] && [ "$1" == "run" ]; then
    echo "Running $EXE"
    ./$OUT_DIR/$EXE &
    echo "Game started in background (PID: $!)"
fi 