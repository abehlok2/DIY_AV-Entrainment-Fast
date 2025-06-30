# Rust Real-Time Audio Backend Plan


This document outlines a plan for implementing a real-time audio generation backend written in **Rust**. The goal is to mirror the features of the Python `sound_creator.py` engine while providing low latency output that can be controlled from a web interface.

## Objectives

1. **Real-Time Performance** – Use efficient DSP code in Rust and the `cpal` crate to stream audio with minimal latency.
2. **Track JSON Compatibility** – Parse the same JSON schema used by the GUI editor so existing projects work without modification.
3. **Modular Synth Functions** – Implement Rust versions of the synths in `src/audio/synth_functions`. Each synth provides a `process(&mut [f32], &mut [f32], sample_rate)` style API.
4. **Web API** – Expose an async HTTP/WebSocket server (using `warp` or `axum`) to control playback.
5. **Extensibility** – Keep modules independent to allow new generators and parameters.

## Proposed Architecture

- **Crate `rust_audio`**
  - `models` module defines `Track`, `Step`, and `Voice` structs using `serde` for JSON parsing.
  - `synth` module contains implementations such as `binaural_beat` and others.
  - `engine` module manages streaming buffers and scheduling using `cpal` for audio output.
  - `api` module optionally exposes HTTP/WebSocket endpoints for control.

## Implementation Steps

1. **Data Structures** – Implement `Track`, `Step`, and `Voice` structs. ✅
2. **Basic Synth Example** – Port the `binaural_beat` synth from Python. ✅ (initial version)
3. **Realtime Engine Skeleton** – Create a `RealtimeEngine` struct that opens an output device via `cpal` and streams audio from a loaded `Track`. ✅ (skeleton only)
4. **JSON Loader** – Provide `load_track` function to parse track JSON into the structs. ✅
5. **API Layer** – TODO: implement HTTP/WebSocket control interface.
6. **Additional Synths** – TODO: port remaining synth functions and transitions.
7. **Testing & Optimization** – TODO: verify output accuracy and measure latency.

Progress is documented below and will be updated as features are implemented.
