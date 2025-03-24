#!/bin/bash
set -e # Exit immediately if a command exits with a non-zero status

# Parse command line arguments
REBUILD=0
for arg in "$@"; do
  case $arg in
    --rebuild)
      REBUILD=1
      shift # Remove --rebuild from processing
      ;;
    *)
      # Unknown option
      ;;
  esac
done

# Check if the build directory exists
if [ ! -d "build" ]; then
  echo "[WARNING] Build directory not found."
  ./build.sh
fi

# Check if we need to build
if [ ! -f "build/bin/PrismMain" ] || [ $REBUILD -eq 1 ]; then
  echo "[INFO] Building project..."
  if ! ./build.sh; then
    echo "[ERROR] Build failed."
    exit 1
  fi
fi

# Navigate to the build directory
cd build/bin

# Check if the binary exists
if [ -f "PrismMain" ]; then
  echo "[INFO] Running Prism Engine..."
  ./PrismMain
else
  echo "[ERROR] PrismMain executable not found. Please check build errors."
  exit 1
fi

# Return to the project root
cd ..
echo "[INFO] Application terminated."