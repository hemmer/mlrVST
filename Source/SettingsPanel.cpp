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
#include "mlrVSTGUI.h"

SettingsPanel::SettingsPanel(const Rectangle<int> &bounds,
                             mlrVSTAudioProcessor * const processorPtr,
                             mlrVSTGUI * const editorPtr) :
    // Communication ////////////////////////////////
    processor(processorPtr), pluginUI(editorPtr),
    // Layout ///////////////////////////////////////
    fontSize(7.4f), panelBounds(bounds), menuLF(),
    // Components ///////////////////////////////////
    panelLabel("settings panel label", "Settings"),

    tempoSourceLbl("tempo source", "tempo source"),
    useExternalTempoBtn("Using external tempo"),

    setNumChannelsLbl("num channels", "num channels"),
    selNumChannels(),

    setRampLengthLbl("length of envelope", "length of envelope"),
    rampLengthSldr(),

    oscPrefixLbl("OSC prefix", "OSC prefix"),
    oscPrefixTxtBx("mlrvst"),

    monitorInputsLbl("monitor inputs", "monitor inputs"),
    monitorInputsBtn(""),

    setMonomeSizeLbl("monome size", "monome size"),
    selMonomeSize(),

    setNumSampleStrips("num sample strips", "num sample strips"),
    selNumSampleStrips()
{
    // main panel label
    addAndMakeVisible(&panelLabel);
    panelLabel.setBounds(0, 0, panelBounds.getWidth(), 30);
    panelLabel.setColour(Label::backgroundColourId, Colours::black);
    panelLabel.setColour(Label::textColourId, Colours::white);
    panelLabel.setFont(2.0f * fontSize);

    const int labelWidth = 150;
    const int labelHeight = 20;

    int yPos = 50;

    // select tempo source (internal / external)
    tempoSourceLbl.setBounds(PAD_AMOUNT, yPos, labelWidth, labelHeight);
    setupLabel(tempoSourceLbl);
    addAndMakeVisible(&useExternalTempoBtn);
    useExternalTempoBtn.setBounds(labelWidth + 2 * PAD_AMOUNT, yPos, 150, 20);
    useExternalTempoBtn.addListener(this);
    // load current value from PluginProcessor
    bool useExternalTempo = *static_cast<const bool*>
        (processor->getGlobalSetting(mlrVSTAudioProcessor::sUseExternalTempo));
    useExternalTempoBtn.setToggleState(useExternalTempo, false);
    String tempoBtnText = (useExternalTempo) ? "external tempo" : "internal tempo";
    useExternalTempoBtn.setButtonText(tempoBtnText);
    yPos += PAD_AMOUNT + labelHeight;


    // combobox to select the number of channels
    setupLabel(setNumChannelsLbl);
    setNumChannelsLbl.setBounds(PAD_AMOUNT, yPos, labelWidth, labelHeight);
    addAndMakeVisible(&selNumChannels);
    selNumChannels.addListener(this);
    for(int i = 1; i <= 8; ++i) selNumChannels.addItem(String(i), i);
    selNumChannels.setBounds(labelWidth + 2 * PAD_AMOUNT, yPos, 150, labelHeight);
    const int numChannels = *static_cast<const int*>
        (processor->getGlobalSetting(mlrVSTAudioProcessor::sNumChannels));
    selNumChannels.setSelectedId(numChannels, true);
    selNumChannels.setLookAndFeel(&menuLF);
    yPos += PAD_AMOUNT + labelHeight;


    // choose envelope length
    setupLabel(setRampLengthLbl);
    setRampLengthLbl.setBounds(PAD_AMOUNT, yPos, labelWidth, labelHeight);
    const int initialRampLength = *static_cast<const int*>(processor->getGlobalSetting(mlrVSTAudioProcessor::sRampLength));
    addAndMakeVisible(&rampLengthSldr);
    rampLengthSldr.setBounds(2*PAD_AMOUNT + labelWidth , yPos, labelWidth, labelHeight);
    rampLengthSldr.setRange(0.0, 1000.0, 1.0);
    rampLengthSldr.setValue(initialRampLength, false, false);
    rampLengthSldr.setColour(Slider::textBoxTextColourId, Colours::white);
    rampLengthSldr.setColour(Slider::thumbColourId, Colours::grey);
    rampLengthSldr.setColour(Slider::backgroundColourId, Colours::grey.darker());
    rampLengthSldr.addListener(this);
    rampLengthSldr.setLookAndFeel(&menuLF);
    rampLengthSldr.setSliderStyle(Slider::LinearBar);
    yPos += PAD_AMOUNT + labelHeight;


    // set the OSC prefix
    addAndMakeVisible(&oscPrefixLbl);
    setupLabel(oscPrefixLbl);
    oscPrefixLbl.setBounds(PAD_AMOUNT, yPos, labelWidth, labelHeight);
    addAndMakeVisible(&oscPrefixTxtBx);
    oscPrefixTxtBx.setBounds(labelWidth + 2*PAD_AMOUNT, yPos, 150, labelHeight);
    oscPrefixTxtBx.addListener(this);
    oscPrefixTxtBx.setEnabled(true);
    const String currentPrefix = *static_cast<const String*>
        (processor->getGlobalSetting(mlrVSTAudioProcessor::sOSCPrefix));
    DBG("settings panel: prefix " << currentPrefix);

    // TODO: fonts freak out here
    // oscPrefixTxtBx.setText("test", false);
    yPos += PAD_AMOUNT + labelHeight;


    // are we including sound from host?
    setupLabel(monitorInputsLbl);
    monitorInputsLbl.setBounds(PAD_AMOUNT, yPos, labelWidth, labelHeight);
    addAndMakeVisible(&monitorInputsBtn);
    monitorInputsBtn.setBounds(labelWidth + 2*PAD_AMOUNT, yPos, 150, labelHeight);
    monitorInputsBtn.addListener(this);
    // load current value from PluginProcessor
    const bool monitorInputs = *static_cast<const bool*>
        (processor->getGlobalSetting(mlrVSTAudioProcessor::sMonitorInputs));
    monitorInputsBtn.setToggleState(monitorInputs, false);
    String monitorBtnText = (monitorInputs) ? "enabled" : "disabled";
    monitorInputsBtn.setButtonText(monitorBtnText);

    yPos += PAD_AMOUNT + labelHeight;

    // what dimension device are we using
    setupLabel(setMonomeSizeLbl);
    setMonomeSizeLbl.setBounds(PAD_AMOUNT, yPos, labelWidth, labelHeight);

    addAndMakeVisible(&selMonomeSize);

    selMonomeSize.addListener(this);
    selMonomeSize.addItem("8x8", mlrVSTAudioProcessor::eightByEight);
    selMonomeSize.addItem("8x16", mlrVSTAudioProcessor::eightBySixteen);
    selMonomeSize.addItem("16x8", mlrVSTAudioProcessor::sixteenByEight);
    selMonomeSize.addItem("16x16", mlrVSTAudioProcessor::sixteenBySixteen);

    selMonomeSize.setBounds(labelWidth + 2 * PAD_AMOUNT, yPos, labelWidth, labelHeight);

    const int monomeSize = *static_cast<const int*>
        (processor->getGlobalSetting(mlrVSTAudioProcessor::sMonomeSize));

    selMonomeSize.setSelectedId(monomeSize, true);
    yPos += PAD_AMOUNT + labelHeight;


    setupLabel(setNumSampleStrips);
    setMonomeSizeLbl.setBounds(PAD_AMOUNT, yPos, labelWidth, labelHeight);

    addAndMakeVisible(&selNumSampleStrips);
    selNumSampleStrips.addListener(this);
    const int numSampleStrips = *static_cast<const int*>
        (processor->getGlobalSetting(mlrVSTAudioProcessor::sNumSampleStrips));

    for (int s = 0; s < 15; ++s)
        selNumSampleStrips.addItem(String(s), s+1);

    selNumSampleStrips.setBounds(labelWidth + 2 * PAD_AMOUNT, yPos, labelWidth, labelHeight);
    selNumSampleStrips.setSelectedId(numSampleStrips+1, true);
    yPos += PAD_AMOUNT + labelHeight;


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
        pluginUI->updateGlobalSetting(mlrVSTAudioProcessor::sUseExternalTempo, &useExternalTempo);
    }
    else if (btn == &monitorInputsBtn)
    {
        bool monitoringInputs = monitorInputsBtn.getToggleState();
        String monitorBtnText = (monitoringInputs) ? "enabled" : "disabled";
        monitorInputsBtn.setButtonText(monitorBtnText);
        pluginUI->updateGlobalSetting(mlrVSTAudioProcessor::sMonitorInputs, &monitoringInputs);
    }
}

void SettingsPanel::comboBoxChanged(ComboBox *box)
{
    if (box == &selNumChannels)
    {
        const int newNumChannels = box->getSelectedId();
        DBG("Number of channels changed to: " + String(newNumChannels));

        // update the global setting
        pluginUI->updateGlobalSetting(mlrVSTAudioProcessor::sNumChannels, &newNumChannels);
    }
    else if (box == &selMonomeSize)
    {
        const int newMonomeSize = box->getSelectedId();
        DBG("New size: " + String(newMonomeSize));

        // update the global setting
        pluginUI->updateGlobalSetting(mlrVSTAudioProcessor::sMonomeSize, &newMonomeSize);
    }
    else if (box == &selNumSampleStrips)
    {
        const int newNumSampleStrips = box->getSelectedId() - 1;
        DBG("New num sample strips: " + String(newNumSampleStrips));

        // let the UI repaint, it will then update the global setting
        pluginUI->buildSampleStripControls(newNumSampleStrips);
    }
}

void SettingsPanel::textEditorChanged(TextEditor &editor)
{
    String newPrefix = editor.getText();
    pluginUI->updateGlobalSetting(mlrVSTAudioProcessor::sOSCPrefix, &newPrefix);
    DBG(newPrefix);
}

void SettingsPanel::textEditorReturnKeyPressed (TextEditor &editor)
{
    String newPrefix = editor.getText();
    pluginUI->updateGlobalSetting(mlrVSTAudioProcessor::sOSCPrefix, &newPrefix);
    DBG(newPrefix);
}

void SettingsPanel::sliderValueChanged(Slider *sldr)
{
    if (sldr == &rampLengthSldr)
    {
        // let the processor know the new value
        int newRampLength = (int) rampLengthSldr.getValue();
        processor->updateGlobalSetting(mlrVSTAudioProcessor::sRampLength, &newRampLength);
    }
}