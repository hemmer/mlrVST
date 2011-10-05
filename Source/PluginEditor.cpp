/*
  ==============================================================================

    mlrVST

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
mlrVSTAudioProcessorEditor::mlrVSTAudioProcessorEditor (mlrVSTAudioProcessor* ownerFilter, const int &newNumChannels)
    : AudioProcessorEditor (ownerFilter),
      infoLabel(),
      masterGainSlider("master gain"),
      bpmSlider("bpm slider"), bpmLabel("BPM"),
      selNumChannels("select number of channels"),
      addPresetBtn("save preset", "Save Preset"),
      toggleSetlistBtn("Show Setlist"),
      toggleSettingsBtn("Settings"),
      presetPanelBounds(314, PAD_AMOUNT, 350, 725),
      settingsPanelBounds(314, PAD_AMOUNT, 350, 725),
      presetPanel(presetPanelBounds, this),
      settingsPanel(settingsPanelBounds, this),
	  sampleStripControlArray(),
      waveformControlHeight(95), waveformControlWidth(700),
	  numChannels(newNumChannels), useExternalTempo(true),
      fontSize(7.4f),
      debugButton("loadfile", DrawableButton::ImageRaw),    // debugging stuff
      loadFilesBtn("load files", "LOAD FILES"),
      slidersArray(),
      myLookAndFeel()
{
    // This tells the GUI to use a custom "theme"
    LookAndFeel::setDefaultLookAndFeel(&myLookAndFeel);

    DBG("GUI loaded");
    setSize(GUI_WIDTH, GUI_HEIGHT);

    addAndMakeVisible(&bpmSlider);
    bpmSlider.setBounds(PAD_AMOUNT, PAD_AMOUNT, 200, 30);
    bpmSlider.setColour(Slider::backgroundColourId, Colours::black.withAlpha(0.3f));
    bpmSlider.setSliderStyle(Slider::LinearBar);
    bpmSlider.setRange(20.0, 300.0, 0.01);
    bpmSlider.setTextBoxIsEditable(true);
    bpmSlider.setEnabled(false);
    bpmSlider.setValue(120.0);
    bpmSlider.addListener(this);

    addAndMakeVisible(&bpmLabel);
    bpmLabel.setBounds(500, 50, 50, 30);
    bpmLabel.setFont(2*fontSize);
    bpmLabel.setColour(Label::textColourId, Colours::black);

    // add a label that will display the current timecode and status..
    addAndMakeVisible(&infoLabel);
    infoLabel.setColour(Label::textColourId, Colours::black);
	infoLabel.setBounds(10, getHeight() - 25, 400, 25);

    // useful UI debugging components
    addAndMakeVisible(&debugButton);
	debugButton.addListener(this);
	debugButton.setBackgroundColours(Colours::blue, Colours::black);
	debugButton.setBounds(50, 300, 50, 25);
    
    addAndMakeVisible(&loadFilesBtn);
	loadFilesBtn.addListener(this);
    loadFilesBtn.setColour(TextButton::buttonColourId, Colours::black);
	loadFilesBtn.setBounds(50, 350, 70, 25);

    buildSampleStripControls();

    masterGainSlider.addListener(this);
    buildSliders();

    // combobox to select the number of channels
    addAndMakeVisible(&selNumChannels);
    for(int i = 1; i <= 8; ++i) selNumChannels.addItem(String(i), i);
    selNumChannels.addListener(this);
    selNumChannels.setBounds(50, 400, 100, 30);
    // NOTE: false flag forces the number of channels to be (re)built,
    // this is where the individual channel volume controls get added
    selNumChannels.setSelectedId(numChannels, true);



    // Preset associated stuff
    addAndMakeVisible(&addPresetBtn);
    addPresetBtn.addListener(this);
    addPresetBtn.setBounds(130, 350, 70, 25);
    addPresetBtn.setColour(TextButton::buttonColourId, Colours::black);

    addAndMakeVisible(&toggleSetlistBtn);
    toggleSetlistBtn.addListener(this);
    toggleSetlistBtn.setBounds(210, 350, 75, 25);
    toggleSetlistBtn.setColour(TextButton::buttonColourId, Colours::black);

    addChildComponent(&presetPanel);
    presetPanel.setBounds(presetPanelBounds);


    // Settings associated stuff
    addAndMakeVisible(&toggleSettingsBtn);
    toggleSettingsBtn.addListener(this);
    toggleSettingsBtn.setBounds(210, 200, 75, 25);
    toggleSettingsBtn.setColour(TextButton::buttonColourId, Colours::black);

    addChildComponent(&settingsPanel);
    settingsPanel.setBounds(settingsPanelBounds);


    formatManager.registerBasicFormats();
    startTimer(50);
}

mlrVSTAudioProcessorEditor::~mlrVSTAudioProcessorEditor()
{
    sampleStripControlArray.clear(true);

    DBG("GUI destructor finished.");
}

void mlrVSTAudioProcessorEditor::buildSampleStripControls()
{
    // make sure we start from scratch
    sampleStripControlArray.clear(true);

    int numStrips = getProcessor()->getNumSampleStrips();

    // Add SampleStripControls, loading settings from the corresponding SampleStrip
    for(int i = 0; i < numStrips; ++i)
    {
        // this is passed to the SampleStripControl to allow data to be stored
        SampleStrip * currentStrip = getProcessor()->getSampleStrip(i);

        sampleStripControlArray.add(new SampleStripControl(i, waveformControlWidth, 
            waveformControlHeight, numChannels, currentStrip, this));
        
        int stripX = getWidth() - waveformControlWidth - PAD_AMOUNT;
        int stripY = PAD_AMOUNT + i * (waveformControlHeight + PAD_AMOUNT);
        sampleStripControlArray[i]->setBounds(stripX, stripY, waveformControlWidth, waveformControlHeight);
        addAndMakeVisible( sampleStripControlArray[i] );

        // Programmatically load parameters
        for(int p = SampleStrip::FirstParam; p < SampleStrip::NumGUIParams; ++p)
        {
            const void *newValue = getProcessor()->getSampleStripParameter(p, i);
            sampleStripControlArray[i]->recallParam(p, newValue, true);
        }
        DBG("params loaded for strip #" << i);
    }
}

void mlrVSTAudioProcessorEditor::buildSliders()
{
    // clear any existing sliders
    slidersArray.clear();

    // work out the correct width
    int sliderWidth = (int)(290 / (float) (numChannels + 1));

    // Set master volume first
    addAndMakeVisible(&masterGainSlider);
    masterGainSlider.setSliderStyle(Slider::LinearVertical);
    masterGainSlider.setRange(0.0, 1.0, 0.01);
    masterGainSlider.setValue(0.6);
    masterGainSlider.setBounds(PAD_AMOUNT, 540, sliderWidth, 190);
    masterGainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);

    for(int i = 0; i < numChannels; ++i)
    {
        slidersArray.add(new Slider("channel " + String(i) + " vol"));
        addAndMakeVisible(slidersArray[i]);
        slidersArray[i]->setSliderStyle(Slider::LinearVertical);
        slidersArray[i]->addListener(this);
        slidersArray[i]->setRange(0.0, 1.0, 0.01);
        slidersArray[i]->setBounds(PAD_AMOUNT + (i + 1) * sliderWidth, 540, sliderWidth, 190);
        slidersArray[i]->setValue(getProcessor()->getChannelProcessor(i)->getChannelGain());

        Colour sliderColour = getProcessor()->getChannelProcessor(i)->getChannelColour();
        slidersArray[i]->setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);
        slidersArray[i]->setColour(Slider::backgroundColourId, sliderColour);
    }

}


//==============================================================================
void mlrVSTAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colours::grey);      // fill with background colour 
}


//==============================================================================
// This timer periodically checks whether any of the filter's
// parameters have changed. In pratical terms, this is usually 
// to see if the host has modified the parameters. 
void mlrVSTAudioProcessorEditor::timerCallback()
{
    mlrVSTAudioProcessor* ourProcessor = getProcessor();

    if (useExternalTempo)
    {
        AudioPlayHead::CurrentPositionInfo newPos(ourProcessor->lastPosInfo);

        if (lastDisplayedPosition != newPos) bpmSlider.setValue(newPos.bpm);
    }

    // TODO: use mlrVSTAudioProcessor::getParameter
    // instead of accessing member variables directly?
    masterGainSlider.setValue(ourProcessor->masterGain, false);

    // check the if the channel volumes have changed
    for(int i = 0; i < slidersArray.size(); ++i)
        slidersArray[i]->setValue( ourProcessor->channelGains[i] );
    
    //// Update sample strip parameters
    //// TODO maybe change callback?
    //for (int strip = 0; strip < sampleStripControlArray.size(); ++strip)
    //{
    //    for(int p = SampleStrip::FirstParam; p < SampleStrip::NumGUIParams; ++p)
    //    {
    //        const void *newValue = getProcessor()->getSampleStripParameter(p, strip);
    //        sampleStripControlArray[strip]->recallParam(p, newValue, false);
    //    }
    //}
}

// This is our Slider::Listener callback, when the user drags a slider.
void mlrVSTAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &masterGainSlider)
    {
        // It's vital to use setParameterNotifyingHost to change any parameters that are automatable
        // by the host, rather than just modifying them directly, otherwise the host won't know
        // that they've changed.
        getProcessor()->setParameterNotifyingHost(mlrVSTAudioProcessor::masterGainParam,
                                                   (float) masterGainSlider.getValue());
    }
    else if (slider == &bpmSlider)
    {
        double newBPM = bpmSlider.getValue();
        getProcessor()->updateGlobalSetting(mlrVSTAudioProcessor::sCurrentBPM, &newBPM);
    }

    // check the channel volume notifications
    for(int i = 0; i < slidersArray.size(); ++i)
    {
        if (slider == slidersArray[i])
        {
            jassert(i < 8);     // we should not have more than 8 channels
            float newChannelGainValue = (float) slidersArray[i]->getValue();

            // let host know about the new value
            // channel0GainParam is the first channel id, so +i to access the rest
            getProcessor()->setParameterNotifyingHost(mlrVSTAudioProcessor::channel0GainParam + i,
                                                      newChannelGainValue);
            // and update the appropriate channel
            getProcessor()->getChannelProcessor(i)->setChannelGain(newChannelGainValue);
            // NEEEEED to update channel processor HERE
        }
    }
   
}

void mlrVSTAudioProcessorEditor::buttonClicked(Button* btn)
{
    if(btn == &toggleSetlistBtn)
    {
        // Toggle the setlist manager panel
        bool currentlyVisible = presetPanel.isVisible();
        String setlistBtnText = (currentlyVisible) ? "Show Setlist" : "Hide Setlist";
        toggleSetlistBtn.setButtonText(setlistBtnText);
        presetPanel.setVisible(!currentlyVisible);

        // Force existing panels to close
        settingsPanel.setVisible(false);
        toggleSettingsBtn.setToggleState(false, false);
    }

    else if(btn == &toggleSettingsBtn)
    {
        // Toggle the settings panel
        bool currentlyVisible = settingsPanel.isVisible();
        settingsPanel.setVisible(!currentlyVisible);

        // Force existing panels to close
        presetPanel.setVisible(false);
        toggleSetlistBtn.setButtonText("Show Setlist");
        toggleSetlistBtn.setToggleState(false, false);
    }

    else if(btn == &addPresetBtn)
    {

    #if JUCE_MODAL_LOOPS_PERMITTED

        AlertWindow w("Save Preset",
                      "Please enter a name for this preset",
                      AlertWindow::NoIcon);

        w.setColour(AlertWindow::textColourId, Colours::black);
        w.setColour(AlertWindow::outlineColourId, Colours::black);

        w.addTextEditor("text", "preset name", "preset name:");

        w.addButton("ok", 1, KeyPress (KeyPress::returnKey, 0, 0));
        w.addButton("cancel", 0, KeyPress (KeyPress::escapeKey, 0, 0));



        if (w.runModalLoop() != 0) // is they picked 'ok'
        {
            // this is the text they entered..
            String newPresetName(w.getTextEditorContents("text"));

            getProcessor()->savePreset(newPresetName);
        }

        
    #endif

    }

    // USEFUL FOR TESTING
	if(btn == &loadFilesBtn)
    {
        
		FileChooser myChooser ("Please choose a file:", File::getSpecialLocation(File::userDesktopDirectory), "*.wav");
        if(myChooser.browseForMultipleFilesToOpen())
        {	
            Array<File> newFiles = myChooser.getResults();
            for (int i = 0; i < newFiles.size(); ++i)
            {
                File newFile = newFiles[i];
                loadSampleFromFile(newFile);
            }

		}

	}
}

// Combo box handling
void mlrVSTAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &selNumChannels)
    {
        numChannels = comboBoxThatHasChanged->getSelectedId();
        DBG("Number of channels changed to: " + String(numChannels));
        
        // Let the audio processor change the number of processing channels
        getProcessor()->buildChannelProcessorArray(numChannels);

        // we need to rebuild the sliders array
	    buildSliders();

        // let the SampleStrips add the right number of buttons
        for(int i = 0; i < sampleStripControlArray.size(); ++i)
            sampleStripControlArray[i]->setNumChannels(numChannels);
    }
}

static const String timeToTimecodeString (const double seconds)
{
    const double absSecs = fabs (seconds);

    const int hours = (int) (absSecs / (60.0 * 60.0));
    const int mins  = ((int) (absSecs / 60.0)) % 60;
    const int secs  = ((int) absSecs) % 60;

    String s;
    if (seconds < 0)
        s = "-";

    s << String (hours).paddedLeft ('0', 2) << ":"
      << String (mins).paddedLeft ('0', 2) << ":"
      << String (secs).paddedLeft ('0', 2) << ":"
      << String (roundToInt (absSecs * 1000) % 1000).paddedLeft ('0', 3);

    return s;
}

// quick-and-dirty function to format a bars/beats string
static const String ppqToBarsBeatsString (double ppq, double /*lastBarPPQ*/, int numerator, int denominator)
{
    if (numerator == 0 || denominator == 0)
        return "1|1|0";

    const int ppqPerBar = (numerator * 4 / denominator);
    const double beats  = (fmod (ppq, ppqPerBar) / ppqPerBar) * numerator;

    const int bar    = ((int) ppq) / ppqPerBar + 1;
    const int beat   = ((int) beats) + 1;
    const int ticks  = ((int) (fmod (beats, 1.0) * 960.0));

    String s;
    s << bar << '|' << beat << '|' << ticks;
    return s;
}

// Updates the text in our position label.
void mlrVSTAudioProcessorEditor::displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos)
{
    lastDisplayedPosition = pos;
    String displayText;
    displayText.preallocateBytes (128);

    displayText << String (pos.bpm, 2) << " bpm, "
                << pos.timeSigNumerator << '/' << pos.timeSigDenominator
                << "  -  " << timeToTimecodeString (pos.timeInSeconds)
                << "  -  " << ppqToBarsBeatsString (pos.ppqPosition, pos.ppqPositionOfLastBarStart,
                                                    pos.timeSigNumerator, pos.timeSigDenominator);

    if (pos.isRecording)
        displayText << "  (recording)";
    else if (pos.isPlaying)
        displayText << "  (playing)";

    infoLabel.setText(displayText, false);
}


// Setting handling stuff
void mlrVSTAudioProcessorEditor::updateGlobalSetting(const int &parameterID, const void *newValue)
{
    /* First let the processor store the setting (as 
       mlrVSTAudioProcessorEditor will lose these on 
       closing.
    */
    getProcessor()->updateGlobalSetting(parameterID, newValue); 

    // a few settings are of interest to the GUI
    switch (parameterID) 
    {
    case mlrVSTAudioProcessor::sUseExternalTempo :
        useExternalTempo = *static_cast<const bool*>(newValue);
        bpmSlider.setEnabled(!useExternalTempo);
        break;
    }
}

const void* mlrVSTAudioProcessorEditor::getGlobalSetting(const int &parameterID, const void *newValue) const
{ 
    return getProcessor()->getGlobalSetting(parameterID);
}
