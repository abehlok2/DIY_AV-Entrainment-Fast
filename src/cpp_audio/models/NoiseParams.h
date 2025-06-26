
#pragma once

#include <juce_core/juce_core.h>

//==============================================================================
/**
    A data structure to hold all the parameters for generating a noise preset.
    This struct is used to pass configuration data to the noise generation
    functions and to serialize the settings to and from a JSON format.
*/
struct NoiseParams {
  //==============================================================================
  // Nested struct for defining a single filter sweep's parameters.
  struct Sweep {
    int startMin = 1000;
    int endMin = 1000;
    int startMax = 10000;
    int endMax = 10000;
    int startQ = 25;
    int endQ = 25;
    int startCasc = 10;
    int endCasc = 10;
  };

  //==============================================================================
  // Member variables for the noise generation parameters.

  double durationSeconds = 60.0;
  int sampleRate = 44100;
  juce::String noiseType = "pink";
  juce::String lfoWaveform = "sine";
  bool transition = false;
  double lfoFreq = 1.0 / 12.0;
  double startLfoFreq = 1.0 / 12.0;
  double endLfoFreq = 1.0 / 12.0;
  int startLfoPhaseOffsetDeg = 0;
  int endLfoPhaseOffsetDeg = 0;
  int startIntraPhaseOffsetDeg = 0;
  int endIntraPhaseOffsetDeg = 0;
  double initialOffset = 0.0;
  double postOffset = 0.0;
  juce::String inputAudioPath;

  // An array to hold one or more filter sweeps.
  juce::Array<Sweep> sweeps;
};
