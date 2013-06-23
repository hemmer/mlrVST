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


class MonomeSelector : public Component
{

public:

    // TODO: remove this if unnecessary
    MonomeSelector(const int &h, const int &w) :
        eightByEightRect(0.0f, 0.0f, w / 2.0f, h / 2.0f),
        eightBySixteenRect(0.0f, 0.0f, w, h / 2.0f),
        sixteenByEightRect(0.0f, 0.0f, w / 2.0f, h),
        sixteenBySixteenRect(0.0f, 0.0f, w, h),
        height(h), width(w)
    {

    };
    ~MonomeSelector() {};

    void paint(Graphics &g)
    {

        g.setColour(Colours::white);

        g.drawRoundedRectangle( eightByEightRect, 10.0f, 4.0f);
        g.drawRoundedRectangle( eightBySixteenRect, 10.0f, 4.0f);

    }


private:

    Rectangle<float> eightByEightRect, eightBySixteenRect;
    Rectangle<float> sixteenByEightRect, sixteenBySixteenRect;

    const int height, width;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MonomeSelector);

};


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
    const Font defaultFont;
    const Rectangle<int> &panelBounds;
    overrideLookandFeel overLF;

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
        lbl.setFont(defaultFont);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel);
};




#endif  // __SETTINGSPANEL_H_726D4579__
