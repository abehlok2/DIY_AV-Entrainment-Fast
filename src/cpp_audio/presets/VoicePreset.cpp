#include "VoicePreset.h"

using namespace juce;

static var namedValueSetToVar(const NamedValueSet& set)
{
    auto* obj = new DynamicObject();
    for (const auto& p : set)
        obj->setProperty(p.name, p.value);
    return var(obj);
}

static NamedValueSet varToNamedValueSet(const var& v)
{
    NamedValueSet set;
    if (auto* obj = v.getDynamicObject())
    {
        for (const auto& p : obj->getProperties())
            set.set(p.name, p.value);
    }
    return set;
}

bool saveVoicePreset(const VoicePreset& preset, const File& file)
{
    File target = file;
    if (target.getFileExtension() != VOICE_FILE_EXTENSION)
        target = target.withFileExtension(VOICE_FILE_EXTENSION);

    auto* obj = new DynamicObject();
    obj->setProperty("synth_function_name", preset.synthFunctionName);
    obj->setProperty("is_transition", preset.isTransition);
    obj->setProperty("params", namedValueSetToVar(preset.params));
    if (!preset.volumeEnvelope.isVoid())
        obj->setProperty("volume_envelope", preset.volumeEnvelope);
    obj->setProperty("description", preset.description);

    var root(obj);
    String json = JSON::toString(root, true);
    return target.replaceWithText(json);
}

VoicePreset loadVoicePreset(const File& file)
{
    VoicePreset preset;
    if (!file.existsAsFile())
        return preset;

    var root = JSON::parse(file.loadFileAsString());
    if (auto* obj = root.getDynamicObject())
    {
        preset.synthFunctionName = obj->getProperty("synth_function_name").toString();
        preset.isTransition = obj->getProperty("is_transition");
        preset.params = varToNamedValueSet(obj->getProperty("params"));
        preset.volumeEnvelope = obj->getProperty("volume_envelope");
        preset.description = obj->getProperty("description").toString();
    }
    return preset;
}
