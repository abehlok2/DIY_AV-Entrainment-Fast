# Audio Generation Details

The `synth_functions/sound_creator.py` script provides the engine for generating audio tracks defined in the GUI. It offers a modular system for combining different synthesis techniques.

## Design

### Framework

The GUI dynamically discovers available synth functions from `sound_creator.SYNTH_FUNCTIONS`. Each function generates a segment of stereo audio based on parameters passed from the GUI (`duration`, `sample_rate`, `params` dictionary).

### Parameter Parsing

Default values for synth functions are automatically parsed from comments within the `sound_creator.py` source code using `inspect` and `ast`, simplifying the addition of new synths.

### Transitions

Most synth functions include a `_transition` variant (e.g., `basic_am_transition`). When selected in the GUI via the "Is Transition?" checkbox, these functions smoothly interpolate key parameters (like frequency, modulation depth, etc.) from a `startValue` to an `endValue` over the duration of the step.

### Track Assembly

The `generate_audio` function orchestrates the process:

  1. Iterates through each step defined in the sequence.
  2. For each step, iterates through all defined voices.
  3. Calls the appropriate synth function (standard or transition) for each voice with its parameters.
  4. Mixes the audio generated by all voices within that step.
  5. Concatenates the audio from each step, applying a phase-aligned crossfade (curve selectable in Preferences, duration set in Global Settings) between steps to ensure smooth, artifact-free transitions.
  6. Applies a safety limiter to the final mixed track.
  7. Normalizes the audio to the **Target Output Amplitude** (0-1.0) specified in Preferences (default `0.25`).
  8. Saves the result as a 16-bit stereo file in the format specified by the output filename (`.wav`, `.flac`, or `.mp3`).

## Functions

Available Synth Functions:

* `binaural_beat`: Generates classic binaural beats by presenting slightly different frequencies to the left and right channels (`baseFreq`, `beatFreq`). Pan is ignored.
* `isochronic_tone`: Creates a pulsing tone (carrier at `baseFreq`) that turns on and off at the `beatFreq`, using a configurable trapezoidal envelope (`rampPercent`, `gapPercent`). Can be panned (`pan`).
* `basic_am`: Simple Amplitude Modulation where a carrier sine wave (`carrierFreq`) is modulated by an LFO (`modFreq`, `modDepth`).
* `fsam_filter_bank`: Frequency-Selective Amplitude Modulation. Filters a noise source (white, pink, or brown) into a specific band (`filterCenterFreq`, `filterRQ`) and modulates only that band with an LFO (`modFreq`, `modDepth`), mixing it back with the rest of the noise.
* `rhythmic_waveshaping`: Modulates the amplitude of a carrier (`carrierFreq`) with an LFO (`modFreq`, `modDepth`) before applying tanh waveshaping (`shapeAmount`).
* `additive_phase_mod`: Two sine waves (fundamental `fundFreq`, 2nd harmonic `h2Amp`). The phase of the second harmonic is modulated by an LFO (`modFreq`, `modDepth`).
* `stereo_am_independent`: Independent AM for left/right channels. Uses slightly detuned carriers (`carrierFreq`, `stereo_width_hz`) and independent LFOs (`modFreqL/R`, `modDepthL/R`, `modPhaseL/R`).
* `noise_am`: Modulates a carrier sine wave (`carrierFreq`) using filtered noise (white/pink/brown, low-pass filtered at `modFilterFreq`) as the modulator source (`modDepth`).
* `wave_shape_stereo_am`: Combines rhythmic waveshaping and stereo AM into one complex voice.
* `spatial_angle_modulation` (*Requires external `audio_engine` module*): Uses Spatial Angle Modulation techniques to simulate a moving sound source. Parameters include `pathShape`, `pathRadius`, `arcStartDeg`, `arcEndDeg`.
* `rhythmic_granular_rate`: Currently a placeholder, does not generate sound.

## Core Utilities

`sound_creator.py` also includes essential DSP building blocks:

* Sine wave generation (constant and varying frequency via phase accumulation).
* Noise generation (white, pink via filtering/colorednoise lib, brown via integration).
* Butterworth filters (bandpass, band-reject, lowpass).
* Envelope generators (ADSR, Linen, Linear Fade).
* Stereo Panning (`pan2` using equal power law).
* Safety Limiter (hard clipping).

## GUI Overview
![Sequence Editor GUI with Voice Editor Dialog](https://github.com/user-attachments/assets/f39bcc5c-3505-4803-b201-8d2f05d44d3c)

The voice editor dialog includes buttons to **Load Preset** and **Save Preset** so common voice configurations can be reused across projects.

### Step Tester Preview
The editor includes a "Test Step Preview" panel for quickly auditioning a single
step without generating the entire track. When a step is selected, the preview
duration adapts to the step length:

* If the step is shorter than 180&nbsp;seconds, the preview plays the entire
  step.
* For longer steps, a 60&nbsp;second excerpt is generated.

This behaviour ensures that very long steps do not delay the preview yet short
steps can still be heard in full.

### Frequency Tester
Use **Tools → Frequency Tester** to quickly audition up to ten binaural voices
without creating a full track. Each voice allows custom base frequency, beat
frequency, and amplitude using the current preference for absolute level or dB.

## C++ Port
A minimal C++ implementation using JUCE lives in `src/cpp_audio`. Build it with CMake and ensure JUCE is available on your system. Core audio engine files now live in `src/cpp_audio/core` and the synthesizer interface header resides in `src/cpp_audio/synths`. The GUI portion remains under `src/cpp_audio/ui` and links directly to the same audio library to provide an integrated editor application.

### Real-Time Prototype
The `RealtimePlayer` target builds a small console application for streaming a track JSON in real time. After configuring the project with CMake you can build and run it like so:

```bash
cmake -B build -S .
cmake --build build --target RealtimePlayer
./build/RealtimePlayer my_track.json
```
It uses JUCE's audio device classes and a double-buffered rendering loop.

The JUCE port now provides a `StepPreviewComponent` that mirrors the Python UI's play/pause/stop controls and time slider for auditioning a single step. It also includes a **Reset** button and labels the currently loaded step alongside the playback time.

An additional helper `loadExternalStepsFromJson` can append steps from another JSON file to an existing `Track`. The JSON must contain a top-level `steps` list, mirroring the "Load External Step" action in the Python editor. The JUCE `StepListPanel` component now exposes a **Load Steps** button to import such files directly into the list.

The console application `diy_av_audio_cpp` now accepts an optional third argument to load such an external step file when generating audio:

```bash
diy_av_audio_cpp input.json output.wav extra_steps.json
```

