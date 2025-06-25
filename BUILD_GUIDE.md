# Build Guide

This document describes how to build the audio components of the project on
**Windows 11** and **Linux**. The repository contains Python scripts and a small
C++ implementation located in `src/cpp_audio` which uses JUCE.

The visual portion and ESP32 firmware are not covered here.

## Prerequisites

1. **Python 3.8+** – install from [python.org](https://www.python.org/) or your
   package manager.
2. **CMake 3.14+** – available from [cmake.org](https://cmake.org/).
3. **JUCE** – the C++ audio project depends on JUCE. Obtain JUCE either by
   cloning the [JUCE repository](https://github.com/juce-framework/JUCE) and
   building/installing it or by using your system's package manager.
   - Set the `JUCE_DIR` environment variable or add the installation prefix to
     `CMAKE_PREFIX_PATH` so that `find_package(JUCE CONFIG)` succeeds.
4. **A C++17 capable compiler** (MSVC on Windows, gcc/clang on Linux).
5. **Optional:** `ninja` build tool for faster builds.

## Building the Python Tools

A Python virtual environment is recommended:

```bash
python -m venv .venv
source .venv/bin/activate   # Windows: .venv\Scripts\activate
pip install -r src/audio/requirements.txt
```

Running the GUI editor or the audio generation scripts only requires the above
packages.

## Building the C++ Audio Application

The C++ implementation in `src/cpp_audio` provides a minimal console program that
renders audio from JSON definitions. The root `CMakeLists.txt` builds this target.

1. Create a build directory and invoke CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

2. The resulting executable `diy_av_audio_cpp` (or `diy_av_audio_cpp.exe` on
   Windows) will be located in `build/src/cpp_audio/Release` if using the default
   generator.

### CMake Presets

A `CMakePresets.json` file is provided for convenience. To build with the default
settings run:

```bash
cmake --preset=default
cmake --build --preset=default
```

Adjust the `juce_dir` cache variable if JUCE is installed in a non-standard
location:

```bash
cmake --preset=default -DJUCE_DIR=/path/to/juce
```

## Notes for Windows

- Install the latest Visual Studio with the **Desktop development with C++**
  workload to obtain MSVC and the necessary Windows SDK.
- Ensure `cmake`, `ninja` (if used), and your compiler are available in the
  command prompt environment (use the "x64 Native Tools" command prompt from
  Visual Studio).

## Notes for Linux

- Install a compiler toolchain (e.g., `build-essential` on Debian/Ubuntu).
- If JUCE is not packaged by your distribution, clone the JUCE repository and
  add the path to `JUCE_DIR` when invoking CMake.

