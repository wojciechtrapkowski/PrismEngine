#!/bin/bash
set -e

# Default build type
BUILD_TYPE="Release"
CLEAN=0

# Parse command line arguments
for arg in "$@"; do
  case $arg in
    --clean)
      CLEAN=1
      shift
      ;;
    --debug)
      BUILD_TYPE="Debug"
      shift
      ;;
    *)
      # Unknown option
      ;;
  esac
done

# Define build directory based on build type
BUILD_DIR="build/$BUILD_TYPE"

if [ $CLEAN -eq 1 ]; then
  echo "[INFO] Cleaning build directory $BUILD_DIR..."
  rm -rf "$BUILD_DIR"
  echo "[INFO] Build directory removed."
  exit 0
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure if CMakeCache.txt doesn't exist
if [ ! -f CMakeCache.txt ]; then
  echo "[INFO] Configuring project with build type: $BUILD_TYPE ..."
  if ! cmake ../.. -DCMAKE_BUILD_TYPE=$BUILD_TYPE; then
    echo "[ERROR] Configuration failed"
    exit 1
  fi
fi

# Get number of CPU cores
if [[ "$(uname)" == "Darwin" ]]; then
  CORES=$(sysctl -n hw.logicalcpu)
else
  CORES=$(nproc 2>/dev/null || echo 2)
fi

# Build the project
echo "[INFO] Building project ($BUILD_TYPE) with $CORES cores..."
if ! cmake --build . -- -j${CORES}; then
  echo "[ERROR] Build failed"
  exit 1
fi

cd ../..
echo "[SUCCESS] Build ($BUILD_TYPE) completed. Executable is in $BUILD_DIR/bin."