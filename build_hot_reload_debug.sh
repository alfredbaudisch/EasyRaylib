#!/bin/bash -eu

echo "Building hot reload (Debug) with CMake..."

mkdir -p build_debug

cd build_debug
cmake -DHOT_RELOAD=ON -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build complete!"
echo "  Executable: $(pwd)/hot_reload/GameProject_hot_reload"
echo "  Game library: $(pwd)/hot_reload/game.dylib (macOS) or game.so (Linux)"
echo ""