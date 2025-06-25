#include "StepPreviewComponent.h"

StepPreviewComponent::StepPreviewComponent(juce::AudioDeviceManager& dm)
    : previewer(dm)
{
    addAndMakeVisible(playPauseButton);
    playPauseButton.addListener(this);

    addAndMakeVisible(stopButton);
    stopButton.addListener(this);

    addAndMakeVisible(positionSlider);
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.addListener(this);

    addAndMakeVisible(timeLabel);
    timeLabel.setJustificationType(juce::Justification::centred);
}

void StepPreviewComponent::loadStep(const Step& step, const GlobalSettings& settings, double previewDuration)
{
    previewer.loadStep(step, settings, previewDuration);
    positionSlider.setRange(0.0, previewer.getLength(), 0.001);
    positionSlider.setValue(0.0);
    updateTimeLabel();
}

void StepPreviewComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
    auto top = area.removeFromTop(24);
    playPauseButton.setBounds(top.removeFromLeft(60));
    stopButton.setBounds(top.removeFromLeft(60));
    timeLabel.setBounds(top);
    area.removeFromTop(4);
    positionSlider.setBounds(area);
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
    }
}

void StepPreviewComponent::sliderValueChanged(juce::Slider* s)
{
    if (s == &positionSlider && ! s->isMouseButtonDown())
        previewer.setPosition(s->getValue());
}

void StepPreviewComponent::timerCallback()
{
    positionSlider.setValue(previewer.getPosition(), juce::dontSendNotification);
    updateTimeLabel();
    if (! previewer.isPlaying())
        stopTimer();
}

void StepPreviewComponent::updateTimeLabel()
{
    auto pos = previewer.getPosition();
    auto len = previewer.getLength();
    timeLabel.setText(juce::String(pos, 1) + " / " + juce::String(len, 1), juce::dontSendNotification);
}

