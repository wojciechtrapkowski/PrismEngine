#!/bin/bash
set -e # Exit immediately if a command exits with a non-zero status

# Parse command line arguments
CLEAN=0
for arg in "$@"; do
  case $arg in
    --clean)
      CLEAN=1
      shift # Remove --clean from processing
      ;;
    *)
      # Unknown option
      ;;
  esac
done

# Clean if requested
if [ $CLEAN -eq 1 ]; then
  echo "[INFO] Cleaning build directory..."
  rm -rf build
  echo "[INFO] Build directory removed."
  exit 0
fi

# Create build directory
mkdir -p build

# Change to the build directory
cd build

# Only reconfigure if CMakeCache.txt does not exist
if [ ! -f CMakeCache.txt ]; then
  echo "[INFO] Configuring project..."
  if ! cmake .. -DCMAKE_BUILD_TYPE=Debug; then
    echo "[ERROR] Configuration failed"
    exit 1
  fi
fi
# Get number of CPU cores for parallel builds
if [[ "$(uname)" == "Darwin" ]]; then
  # macOS
  CORES=$(sysctl -n hw.logicalcpu)
else
  # Linux and others
  CORES=$(nproc 2>/dev/null || echo 2)
fi

# Build the project
echo "[INFO] Building project with $CORES cores..."
if ! cmake --build . -- -j${CORES}; then
  echo "[ERROR] Build failed"
  exit 1
fi

# Return to the project root
cd ..
echo "[SUCCESS] Build completed. You can find the executable in the build/bin directory."