#include "StepPreviewComponent.h"
#include <cmath>

StepPreviewComponent::StepPreviewComponent(juce::AudioDeviceManager& dm)
    : previewer(dm)
{
    addAndMakeVisible(&playPauseButton);
    playPauseButton.addListener(this);
    playPauseButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.addListener(this);
    stopButton.setEnabled(false);

    addAndMakeVisible(&resetButton);
    resetButton.addListener(this);

    addAndMakeVisible(&positionSlider);
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.addListener(this);

    addAndMakeVisible(&timeLabel);
    timeLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(&stepLabel);
    stepLabel.setJustificationType(juce::Justification::centred);
    stepLabel.setText("No step loaded", juce::dontSendNotification);
}

void StepPreviewComponent::loadStep(const Step& step, const GlobalSettings& settings, double previewDuration)
{
    previewer.stop();
    playPauseButton.setButtonText("Play");
    previewer.loadStep(step, settings, previewDuration);
    loadedStepName = step.description;
    stepReady = false;
    playPauseButton.setEnabled(false);
    stopButton.setEnabled(false);
    positionSlider.setRange(0.0, 1.0, 0.001);
    positionSlider.setValue(0.0);
    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    stepLabel.setText("Loading: " + loadedStepName, juce::dontSendNotification);
    startTimerHz(30);
}

void StepPreviewComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
    auto top = area.removeFromTop(24);
    playPauseButton.setBounds(top.removeFromLeft(60));
    stopButton.setBounds(top.removeFromLeft(60));
    resetButton.setBounds(top.removeFromLeft(60));
    top.removeFromLeft(4);
    stepLabel.setBounds(top);

    area.removeFromTop(4);
    positionSlider.setBounds(area.removeFromTop(24));
    area.removeFromTop(4);
    timeLabel.setBounds(area.removeFromTop(24));
}

void StepPreviewComponent::buttonClicked(juce::Button* b)
{
    if (b == &playPauseButton)
    {
        if (previewer.isPlaying())
        {
            previewer.pause();
            playPauseButton.setButtonText("Play");
        }
        else
        {
            previewer.play();
            playPauseButton.setButtonText("Pause");
            startTimerHz(30);
        }
    }
    else if (b == &stopButton)
    {
        previewer.stop();
        playPauseButton.setButtonText("Play");
        positionSlider.setValue(0.0);
        updateTimeLabel();
        if (loadedStepName.isNotEmpty())
            stepLabel.setText("Ready: " + loadedStepName, juce::dontSendNotification);
    }
    else if (b == &resetButton)
    {
        reset();
    }
}

void StepPreviewComponent::sliderValueChanged(juce::Slider* s)
{
    if (s == &positionSlider && ! s->isMouseButtonDown())
    {
        previewer.setPosition(s->getValue());
        updateTimeLabel();
    }
}

void StepPreviewComponent::timerCallback()
{
    if (!stepReady)
    {
        if (previewer.isReady())
        {
            stepReady = true;
            playPauseButton.setEnabled(true);
            stopButton.setEnabled(true);
            positionSlider.setRange(0.0, previewer.getLength(), 0.001);
            updateTimeLabel();
            stepLabel.setText("Ready: " + loadedStepName, juce::dontSendNotification);
            if (!previewer.isPlaying())
                stopTimer();
        }
        return;
    }

    positionSlider.setValue(previewer.getPosition(), juce::dontSendNotification);
    updateTimeLabel();
    if (! previewer.isPlaying())
    {
        stopTimer();
    }
}

void StepPreviewComponent::updateTimeLabel()
{
    auto pos = previewer.getPosition();
    auto len = previewer.getLength();
    timeLabel.setText(formatTime(pos) + " / " + formatTime(len), juce::dontSendNotification);
}

juce::String StepPreviewComponent::formatTime(double seconds)
{
    int total = (int)std::round(seconds);
    int mins = total / 60;
    int secs = total % 60;
    return juce::String(mins).paddedLeft('0', 2) + ":" + juce::String(secs).paddedLeft('0', 2);
}

void StepPreviewComponent::reset()
{
    previewer.stop();
    loadedStepName.clear();
    stepReady = false;
    playPauseButton.setButtonText("Play");
    playPauseButton.setEnabled(false);
    stopButton.setEnabled(false);
    positionSlider.setRange(0.0, 1.0, 0.001);
    positionSlider.setValue(0.0);
    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    stepLabel.setText("No step loaded", juce::dontSendNotification);
}

