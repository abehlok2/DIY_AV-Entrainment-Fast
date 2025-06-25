#include "Track.h"
#include "AudioUtils.h"
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>

#define DIY_AV_UI_NO_MAIN
#include "../cpp_ui/main.cpp"

int main (int argc, char* argv[])
{
    return juce::JUCEApplicationBase::main (argc, argv, new DiyAvApplication());
}

