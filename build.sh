#!/bin/bash


mkdir -p build
cd build

# Configure and build
cmake ../src/ -DCMAKE_BUILD_TYPE=Release
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "Build complete!"
echo "Run with: ./build/mnist"

