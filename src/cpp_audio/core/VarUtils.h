#pragma once
#include <juce_core/juce_core.h>

inline juce::var getPropertyWithDefault(const juce::DynamicObject *obj,
                                        const juce::Identifier &name,
                                        const juce::var &defaultValue) {
  if (obj != nullptr && obj->hasProperty(name))
    return obj->getProperty(name);
  return defaultValue;
}

inline juce::var withDefault(const juce::var &value,
                             const juce::var &defaultValue) {
  return value.isVoid() ? defaultValue : value;
}

inline juce::var namedValueSetToVar(const juce::NamedValueSet &set) {
  auto *obj = new juce::DynamicObject();
  for (const auto &p : set)
    obj->setProperty(p.name, p.value);
  return juce::var(obj);
}

inline juce::NamedValueSet varToNamedValueSet(const juce::var &v) {
  juce::NamedValueSet set;
  if (auto *obj = v.getDynamicObject()) {
    for (const auto &p : obj->getProperties())
      set.set(p.name, p.value);
  }
  return set;
}
