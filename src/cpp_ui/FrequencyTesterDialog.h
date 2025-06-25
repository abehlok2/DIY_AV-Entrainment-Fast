
#ifndef DIY_AV_UI_FREQUENCY_TESTER_DIALOG_H
#define DIY_AV_UI_FREQUENCY_TESTER_DIALOG_H
#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
std::unique_ptr<juce::Component> createFrequencyTesterDialog(juce::AudioDeviceManager&);
#endif
