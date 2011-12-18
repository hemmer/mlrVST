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
#include "mlrVSTLookAndFeel.h"

/* Forward declaration to set up pointer arrangement 
   to allow settings panel access to the UI */
class mlrVSTAudioProcessorEditor;


class SettingsPanel : 
    public Component,
    public ButtonListener,
    public ComboBoxListener,
    public TextEditorListener
{

public:

    SettingsPanel(const Rectangle<int> &bounds,
                mlrVSTAudioProcessorEditor * const owner);

    void buttonClicked(Button *btn);
    void comboBoxChanged(ComboBox *box);
    void textEditorChanged(TextEditor &editor);
    void textEditorReturnKeyPressed (TextEditor &editor);
    void paint(Graphics& g);

private:
    
    menuLookandFeel menuLF;

    // Pointer to parent GUI component
    mlrVSTAudioProcessorEditor * const mlrVSTEditor;
    // Main header label
    Label panelLabel;

    void setupLabel(Label &lbl)
    {
        lbl.setColour(Label::backgroundColourId, Colours::white);
        lbl.setColour(Label::textColourId, Colours::black);
        lbl.setFont(fontSize);
    }

    Label tempoSourceLbl;
    ToggleButton useExternalTempoBtn;
    
    Label setNumChannelsLbl;
    ComboBox selNumChannels;

    Label oscPrefixLbl;
    TextEditor oscPrefixTxtBx;

    ToggleButton monitorInputsBtn;
    Label monitorInputsLbl;

    const float fontSize;
    const Rectangle<int> &panelBounds;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel);
};



#endif  // __SETTINGSPANEL_H_726D4579__
