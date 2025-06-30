# Rust Real-Time Audio Backend Plan

This document tracks progress on implementing a real-time audio backend in Rust. The goal is to replicate features of `sound_creator.py` for low‑latency playback.

## Implemented
- Created new Cargo crate `realtime_audio_rs` with modules for `models`, `synths` and `engine`.
- Added basic data structures (`Track`, `Step`, `Voice`, `GlobalSettings`) using `serde` for JSON loading.
- Implemented a simple `binaural_beat` generator.
- Added `RealtimeEngine` that loads a track and plays each step using `rodio`.
- Provided a command line binary that accepts a track JSON file and plays it.

## TODO
- Support additional synth functions from Python reference (`isochronic_tone`, etc.).
- Implement parameter transitions and crossfade logic.
- Add safety limiter and normalization stages.
- Expose a web API (e.g., via WebSocket) for remote control.
- Unit tests comparing generated buffers with Python version.
- Documentation updates and build instructions.
