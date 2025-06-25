#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../cpp_audio/StepPreviewer.h"

class StepPreviewComponent : public juce::Component,
                              private juce::Button::Listener,
                              private juce::Slider::Listener,
                              private juce::Timer
{
public:
    explicit StepPreviewComponent(juce::AudioDeviceManager& dm);

    void loadStep(const Step& step, const GlobalSettings& settings, double previewDuration);
    void reset();
    void resized() override;

private:
    void buttonClicked(juce::Button*) override;
    void sliderValueChanged(juce::Slider*) override;
    void timerCallback() override;
    void updateTimeLabel();
    static juce::String formatTime(double seconds);

    StepPreviewer previewer;
    juce::TextButton playPauseButton {"Play"};
    juce::TextButton stopButton {"Stop"};
    juce::TextButton resetButton {"Reset"};
    juce::Slider positionSlider;
    juce::Label timeLabel;
    juce::Label stepLabel;
    juce::String loadedStepName;
};

