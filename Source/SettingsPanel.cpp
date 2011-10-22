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
    PAD_AMOUNT(10),
    useExternalTempoBtn("Using external tempo")
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
}

void SettingsPanel::paint(Graphics &g)
{
    g.fillAll(Colours::grey.withAlpha(0.5f));
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