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
    menuLF(),

    tempoSourceLbl("tempo source", "tempo source"),
    useExternalTempoBtn("Using external tempo"),

    setNumChannelsLbl("num channels", "num channels"),
    selNumChannels("select number of channels"),

    oscPrefixLbl("OSC prefix", "OSC prefix"),
    oscPrefixTxtBx("mlrvst"),

    monitorInputsLbl("monitor inputs", "monitor inputs"),
    monitorInputsBtn("")
{
    addAndMakeVisible(&panelLabel);
    panelLabel.setBounds(0, 0, panelBounds.getWidth(), 30);
    panelLabel.setColour(Label::backgroundColourId, Colours::black);
    panelLabel.setColour(Label::textColourId, Colours::white);
    panelLabel.setFont(2 * fontSize);

    int yPos = 50;
    addAndMakeVisible(&tempoSourceLbl);
    tempoSourceLbl.setBounds(PAD_AMOUNT, yPos, 150, 20);
    setupLabel(tempoSourceLbl);
    addAndMakeVisible(&useExternalTempoBtn);
    useExternalTempoBtn.setBounds(150 + 2 * PAD_AMOUNT, yPos, 150, 20);
    useExternalTempoBtn.addListener(this);

    // load current values of settings from PluginProcessor
    bool useExternalTempo = *static_cast<const bool*>
        (mlrVSTEditor->getGlobalSetting(mlrVSTAudioProcessor::sUseExternalTempo));
    useExternalTempoBtn.setToggleState(useExternalTempo, false);
    String tempoBtnText = (useExternalTempo) ? "external tempo" : "internal tempo";
    useExternalTempoBtn.setButtonText(tempoBtnText);



    // combobox to select the number of channels
    int numChannels = *static_cast<const int*>
        (mlrVSTEditor->getGlobalSetting(mlrVSTAudioProcessor::sNumChannels));

    yPos+=30;

    addAndMakeVisible(&setNumChannelsLbl);
    setNumChannelsLbl.setBounds(PAD_AMOUNT, yPos, 150, 20);
    setupLabel(setNumChannelsLbl);

    addAndMakeVisible(&selNumChannels);
    selNumChannels.addListener(this);
    for(int i = 1; i <= 8; ++i) selNumChannels.addItem(String(i), i);
    selNumChannels.addListener(this);
    selNumChannels.setBounds(150 + 2 * PAD_AMOUNT, yPos, 150, 20);
    selNumChannels.setSelectedId(numChannels, true);
    selNumChannels.setLookAndFeel(&menuLF);



    yPos += PAD_AMOUNT + 20;
    addAndMakeVisible(&oscPrefixLbl);
    setupLabel(oscPrefixLbl);
    oscPrefixLbl.setBounds(PAD_AMOUNT, yPos, 150, 20);
    
    addAndMakeVisible(&oscPrefixTxtBx);
    oscPrefixTxtBx.setBounds(2*PAD_AMOUNT+150, yPos, 150, 20);
    oscPrefixTxtBx.addListener(this);
    oscPrefixTxtBx.setEnabled(true);
    const String currentPrefix = *static_cast<const String*>
        (mlrVSTEditor->getGlobalSetting(mlrVSTAudioProcessor::sOSCPrefix));
    DBG("settings panel: prefix " << currentPrefix);
    // eh?
    //oscPrefixTxtBx.setText("test", false);




    yPos += PAD_AMOUNT + 20;
    addAndMakeVisible(&monitorInputsLbl);
    setupLabel(monitorInputsLbl);
    monitorInputsLbl.setBounds(PAD_AMOUNT, yPos, 150, 20);

    addAndMakeVisible(&monitorInputsBtn);
    monitorInputsBtn.setBounds(2*PAD_AMOUNT+150, yPos, 150, 20);
    monitorInputsBtn.addListener(this);

    // load current values of settings from PluginProcessor
    bool monitorInputs = *static_cast<const bool*>
        (mlrVSTEditor->getGlobalSetting(mlrVSTAudioProcessor::sMonitorInputs));

    monitorInputsBtn.setToggleState(monitorInputs, false);
    String monitorBtnText = (monitorInputs) ? "enabled" : "disabled";
    monitorInputsBtn.setButtonText(monitorBtnText);
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
        String tempoBtnText = (useExternalTempo) ? "external tempo" : "internal tempo";
        useExternalTempoBtn.setButtonText(tempoBtnText);
        mlrVSTEditor->updateGlobalSetting(mlrVSTAudioProcessor::sUseExternalTempo, &useExternalTempo);
    }
    else if (btn == &monitorInputsBtn)
    {
        bool monitoringInputs = monitorInputsBtn.getToggleState();
        String monitorBtnText = (monitoringInputs) ? "enabled" : "disabled";
        monitorInputsBtn.setButtonText(monitorBtnText);
        mlrVSTEditor->updateGlobalSetting(mlrVSTAudioProcessor::sMonitorInputs, &monitoringInputs);
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

void SettingsPanel::textEditorChanged(TextEditor &editor)
{
    String newPrefix = editor.getText();
    mlrVSTEditor->updateGlobalSetting(mlrVSTAudioProcessor::sOSCPrefix, &newPrefix);
    DBG(newPrefix);
}

void SettingsPanel::textEditorReturnKeyPressed (TextEditor &editor)
{
    String newPrefix = editor.getText();
    mlrVSTEditor->updateGlobalSetting(mlrVSTAudioProcessor::sOSCPrefix, &newPrefix);
    DBG(newPrefix);
}
