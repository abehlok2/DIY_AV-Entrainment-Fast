#pragma once
#include <juce_core/juce_core.h>

inline juce::var getPropertyWithDefault(const juce::DynamicObject* obj,
                                        const juce::Identifier& name,
                                        const juce::var& defaultValue)
{
    if (obj != nullptr && obj->hasProperty(name))
        return obj->getProperty(name);
    return defaultValue;
}
