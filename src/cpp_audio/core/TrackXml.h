#pragma once

#include "TrackData.h"
#include <juce_core/juce_core.h>

Track loadTrackFromXml(const juce::File& file);
bool saveTrackToXml(const Track& track, const juce::File& file);
