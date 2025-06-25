#include "VoiceModel.h"
#include <cmath>

VoiceModel::VoiceModel(std::vector<Voice>* voicesIn, juce::TableListBox* ownerIn)
    : voices(voicesIn), owner(ownerIn) {}

int VoiceModel::getNumRows()
{
    return voices ? static_cast<int>(voices->size()) : 0;
}

void VoiceModel::paintRowBackground(juce::Graphics& g, int /*rowNumber*/, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
}

void VoiceModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    if (!voices || rowNumber >= (int)voices->size())
        return;

    const Voice& voice = (*voices)[rowNumber];
    juce::String text;

    if (columnId == 1)
        text = voice.synthFunction;
    else if (columnId == 2)
    {
        if (voice.params.contains("baseFreq"))
            text = formatNumber(voice.params["baseFreq"]);
        else if (voice.params.contains("frequency"))
            text = formatNumber(voice.params["frequency"]);
        else if (voice.params.contains("carrierFreq"))
            text = formatNumber(voice.params["carrierFreq"]);
    }
    else if (columnId == 3)
        text = getBeatFrequency(voice.params, voice.isTransition);
    else if (columnId == 4)
        text = voice.isTransition ? "Yes" : "No";
    else if (columnId == 5)
        text = voice.description;

    g.drawText(text, 0, 0, width, height, juce::Justification::centredLeft, true);
}

juce::String VoiceModel::formatNumber(const juce::var& value) const
{
    if (value.isDouble() || value.isInt())
        return juce::String((double)value, 2);
    return value.toString();
}

juce::String VoiceModel::getBeatFrequency(const juce::NamedValueSet& params, bool isTransition) const
{
    auto contains = [&](const juce::Identifier& id) { return params.contains(id); };
    if (isTransition)
    {
        if (contains("startBeatFreq") && contains("endBeatFreq"))
        {
            double s = (double)params.getWithDefault("startBeatFreq", 0.0);
            double e = (double)params.getWithDefault("endBeatFreq", 0.0);
            if (std::abs(s - e) < 1e-6)
                return juce::String(s, 2);
            return juce::String(s, 2) + "->" + juce::String(e, 2);
        }
        if (contains("startBeatFreq"))
            return formatNumber(params["startBeatFreq"]);
        if (contains("endBeatFreq"))
            return formatNumber(params["endBeatFreq"]);
    }
    if (contains("beatFreq"))
        return formatNumber(params["beatFreq"]);
    return "N/A";
}

void VoiceModel::refresh(std::vector<Voice>* newVoices)
{
    if (newVoices)
        voices = newVoices;
    if (auto* owner = getOwner())
        owner->updateContent();
}
