/*
  ==============================================================================

    SettingsPanel.cpp
    Created: 30 Sep 2011 2:18:46pm
    Author:  Hemmer

    This panel is created by Plugin Editor and its visibilty is toggled to
    display it. It loads settings from PluginProcessor.

  ==============================================================================
*/

#include "SettingsPanel.h"
#include "PluginEditor.h"

SettingsPanel::SettingsPanel(const Rectangle<int> &bounds,
                             mlrVSTAudioProcessorEditor * const owner) :
    mlrVSTEditor(owner),
    panelLabel("settings panel label", "Settings"),
    fontSize(7.4f), panelBounds(bounds),
    PAD_AMOUNT(10), menuLF(),
    useExternalTempoBtn("Using external tempo"),
    setNumChannelsLbl("num channels", "num channels"),
    selNumChannels("select number of channels")
{
    addAndMakeVisible(&panelLabel);
    panelLabel.setBounds(0, 0, panelBounds.getWidth(), 30);
    panelLabel.setColour(Label::backgroundColourId, Colours::black);
    panelLabel.setColour(Label::textColourId, Colours::white);
    panelLabel.setFont(2 * fontSize);

    addAndMakeVisible(&useExternalTempoBtn);
    useExternalTempoBtn.setBounds(PAD_AMOUNT, 50, 150, 20);
    useExternalTempoBtn.addListener(this);

    // load current values of settings from PluginProcessor
    bool useExternalTempo = *static_cast<const bool*>
        (mlrVSTEditor->getGlobalSetting(mlrVSTAudioProcessor::sUseExternalTempo));

    useExternalTempoBtn.setToggleState(useExternalTempo, false);
    String tempoBtnText = (useExternalTempo) ? "Using External Tempo" : "Using Internal Tempo";
    useExternalTempoBtn.setButtonText(tempoBtnText);


    // combobox to select the number of channels
    int numChannels = *static_cast<const int*>
        (mlrVSTEditor->getGlobalSetting(mlrVSTAudioProcessor::sNumChannels));
    DBG(numChannels);

    addAndMakeVisible(&setNumChannelsLbl);
    setNumChannelsLbl.setBounds(PAD_AMOUNT, 80, 150, 20);
    setNumChannelsLbl.setColour(Label::backgroundColourId, Colours::white);
    setNumChannelsLbl.setColour(Label::textColourId, Colours::black);
    setNumChannelsLbl.setFont(fontSize);

    addAndMakeVisible(&selNumChannels);
    selNumChannels.addListener(this);
    for(int i = 1; i <= 8; ++i) selNumChannels.addItem(String(i), i);
    selNumChannels.addListener(this);
    selNumChannels.setBounds(150 + 2 * PAD_AMOUNT, 80, 100, 20);
    // NOTE: false flag forces the number of channels to be (re)built,
    // this is where the individual channel volume controls get added
    selNumChannels.setSelectedId(numChannels, true);
    selNumChannels.setLookAndFeel(&menuLF);
}

void SettingsPanel::paint(Graphics &g)
{
    g.fillAll(Colours::grey.withAlpha(0.9f));
}

void SettingsPanel::buttonClicked(Button *btn)
{
    if (btn == &useExternalTempoBtn)
    {
        bool useExternalTempo = useExternalTempoBtn.getToggleState();
        String tempoBtnText = (useExternalTempo) ? "Using External Tempo" : "Using Internal Tempo";
        useExternalTempoBtn.setButtonText(tempoBtnText);
        mlrVSTEditor->updateGlobalSetting(mlrVSTAudioProcessor::sUseExternalTempo, &useExternalTempo);
    }
}

void SettingsPanel::comboBoxChanged(ComboBox *box)
{
    if (box == &selNumChannels)
    {
        int numChannels = box->getSelectedId();
        DBG("Number of channels changed to: " + String(numChannels));

        // Let the audio processor change the number of audio channels
        mlrVSTEditor->updateGlobalSetting(mlrVSTAudioProcessor::sNumChannels, &numChannels);
    }
}