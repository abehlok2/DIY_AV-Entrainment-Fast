#!/usr/bin/env bash
set -e

# setup.sh - Prepare Ubuntu environment for DIY_AV-Entrainment-Fast
# Installs system packages, clones JUCE, sets up a Python virtual environment,
# and builds the C++ audio application using CMake.

# Determine repository root (directory containing this script)
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_ROOT"

# Install required system packages
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    python3-venv \
    python3-pip \
    portaudio19-dev \
    libsndfile1 \
    ffmpeg

# Clone JUCE if not already present
JUCE_DIR="src/cpp_audio/JUCE"
if [ ! -d "$JUCE_DIR" ]; then
    git clone --depth 1 https://github.com/juce-framework/JUCE.git "$JUCE_DIR"
fi

# Set up Python virtual environment and install Python dependencies
if [ ! -d ".venv" ]; then
    python3 -m venv .venv
fi
source .venv/bin/activate
pip install --upgrade pip
pip install -r src/audio/requirements.txt
deactivate

# Build the C++ audio application
cmake --preset=default
cmake --build --preset=default

cat <<'MSG'

Setup complete. Activate the virtual environment with:
  source .venv/bin/activate
and rerun cmake if you change any source files.

MSG
