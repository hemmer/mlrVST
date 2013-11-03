/*
  ==============================================================================

    Preset.h
    Created: 3 Nov 2013 12:09:29pm
    Author:  hemmer

    This is a static class for saving / loading Presets within mlrVST and
    saving / loading them to / from an external file.

  ==============================================================================
*/

#ifndef PRESET_H_INCLUDED
#define PRESET_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "GlobalSettings.h"


class Preset
{
public:

    // Pass this a pointer to all settings, and get an
    // XmlElement object with all settings required for a preset
    static XmlElement createPreset(const String &presetName,
                                   mlrVSTAudioProcessor * processor,
                                   GlobalSettings *gs);

    // Pass this a preset in XmlElement form and this method
    // will load the values in as required
    static void loadPreset(XmlElement * presetToLoad,
                           mlrVSTAudioProcessor * processor,
                           GlobalSettings *gs);

private:

     JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Preset);
};





#endif  // PRESET_H_INCLUDED
