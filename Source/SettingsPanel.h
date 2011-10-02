/*
  ==============================================================================

    SettingsPanel.h
    Created: 30 Sep 2011 2:18:46pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __SETTINGSPANEL_H_726D4579__
#define __SETTINGSPANEL_H_726D4579__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"


/* Forward declaration to set up pointer arrangement 
   to allow settings panel access to the UI */
class mlrVSTAudioProcessorEditor;


class SettingsPanel : 
    public Component,
    public ButtonListener
{

public:

    SettingsPanel(const Rectangle<int> &bounds,
                mlrVSTAudioProcessorEditor * const owner);

    void buttonClicked(Button *btn);
    void paint(Graphics& g);

private:

    // Pointer to parent GUI component
    mlrVSTAudioProcessorEditor * const mlrVSTEditor;
    // Main header label
    Label panelLabel;

    ToggleButton useExternalTempoBtn;

    const int PAD_AMOUNT;

    const float fontSize;
    const Rectangle<int> &panelBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel);
};



#endif  // __SETTINGSPANEL_H_726D4579__
