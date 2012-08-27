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
#include "mlrVSTLookAndFeel.h"

/* Forward declaration to set up pointer arrangement
   to allow settings panel access to the UI */
class mlrVSTAudioProcessor;
class mlrVSTGUI;


class SettingsPanel : public Component,
                      public ButtonListener,
                      public ComboBoxListener,
                      public SliderListener,
                      public TextEditorListener

{

public:

    SettingsPanel(const Rectangle<int> &bounds,
                  mlrVSTAudioProcessor * const processorPtr,
                  mlrVSTGUI * const editorPtr);

    void buttonClicked(Button *btn);
    void comboBoxChanged(ComboBox *box);
    void sliderValueChanged(Slider *sldr);
    void textEditorChanged(TextEditor &editor);
    void textEditorReturnKeyPressed (TextEditor &editor);
    void paint(Graphics& g);

private:

    // Communication ////////////////////////////////
    // Here we have pointers to the main audio
    // processor for getting global settings and
    // to the UI for updating settings (as these
    // may affect the UI, e.g. changing # of channels
    mlrVSTAudioProcessor * const processor;
    mlrVSTGUI * const pluginUI;

    // Layout ///////////////////////////////////////
    const float fontSize;
    const Rectangle<int> &panelBounds;
    menuLookandFeel menuLF;

    // Components ///////////////////////////////////
    Label panelLabel;       // Main header label

    Label tempoSourceLbl;
    ToggleButton useExternalTempoBtn;

    Label setNumChannelsLbl;
    ComboBox selNumChannels;

    Label setRampLengthLbl;
    Slider rampLengthSldr;

    Label oscPrefixLbl;
    TextEditor oscPrefixTxtBx;

    Label monitorInputsLbl;
    ToggleButton monitorInputsBtn;

    Label setMonomeSizeLbl;
    ComboBox selMonomeSize;

    Label setNumSampleStrips;
    ComboBox selNumSampleStrips;

    void setupLabel(Label &lbl)
    {
        addAndMakeVisible(&lbl);
        lbl.setColour(Label::backgroundColourId, Colours::white);
        lbl.setColour(Label::textColourId, Colours::black);
        lbl.setFont(fontSize);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel);
};



#endif  // __SETTINGSPANEL_H_726D4579__
