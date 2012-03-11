/*
  ==============================================================================

    mlrVST

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
mlrVSTAudioProcessorEditor::mlrVSTAudioProcessorEditor (mlrVSTAudioProcessor* ownerFilter,
                                                        const int &newNumChannels, 
                                                        const int &newNumStrips)
    : AudioProcessorEditor (ownerFilter),
      myLookAndFeel(), menuLF(),

      infoLabel(),
      masterGainSlider("master gain"),
      bpmSlider("bpm slider"), bpmLabel("BPM", "BPM"),
      addPresetBtn("save preset", "Save Preset"),
      toggleSetlistBtn("Show Setlist"),
      toggleSettingsBtn("Settings"),
      presetPanelBounds(294, PAD_AMOUNT, THUMBNAIL_WIDTH / 2, 725),
      settingsPanelBounds(294, PAD_AMOUNT, THUMBNAIL_WIDTH / 2, 725),
      presetPanel(presetPanelBounds, this),
      settingsPanel(settingsPanelBounds, this),
      sampleStripControlArray(), numStrips(newNumStrips),
      waveformControlHeight( (GUI_HEIGHT - numStrips * PAD_AMOUNT) / numStrips),
      waveformControlWidth(THUMBNAIL_WIDTH),
	  numChannels(newNumChannels), useExternalTempo(true),
      fontSize(7.4f),
      debugButton("loadfile", DrawableButton::ImageRaw),    // debugging stuff
      loadFilesBtn("load files", "LOAD FILES"),

      precountLbl("precount", "precount"), recordLengthLbl("length", "length"), bankLbl("bank", "bank"),
      resamplePrecountSldr(), resampleBankSldr(), resampleLengthSldr(),
      recordPrecountSldr(), recordLengthSldr(), recordBankSldr(),
      recordBtn("RECORD", Colours::black, Colours::white),
      resampleBtn("RESAMPLE", Colours::black, Colours::white),
      slidersArray()

{
    MemoryInputStream mis(BinaryData::silkfont, BinaryData::silkfontSize, false);
    typeSilk = new CustomTypeface(mis);
    fontSilk = Font( typeSilk );


    DBG("GUI loaded");
    setSize(GUI_WIDTH, GUI_HEIGHT);

    setWantsKeyboardFocus(true);

    useExternalTempo = *static_cast<const bool*>(getGlobalSetting(mlrVSTAudioProcessor::sUseExternalTempo));

    addAndMakeVisible(&bpmSlider);
    bpmSlider.setBounds(PAD_AMOUNT, 400, 200, 30);
    bpmSlider.setColour(Slider::backgroundColourId, Colours::black.withAlpha(0.3f));
    bpmSlider.setSliderStyle(Slider::LinearBar);
    bpmSlider.setRange(20.0, 300.0, 0.01);
    bpmSlider.setTextBoxIsEditable(true);
    bpmSlider.addListener(this);
    
    if (useExternalTempo)
    {
        bpmSlider.setEnabled(false);
    }
    else 
    {
        double newBPM = *static_cast<const double*>(getGlobalSetting(mlrVSTAudioProcessor::sCurrentBPM));
        bpmSlider.setValue(newBPM);
    }

    addAndMakeVisible(&bpmLabel);
    bpmLabel.setBounds(250, 400, 50, 30);
    bpmLabel.setFont(2*fontSize);
    bpmLabel.setColour(Label::textColourId, Colours::black);
    //bpmLabel.setLookAndFeel(&menuLF);

    // add a label that will display the current timecode and status..
    addAndMakeVisible(&infoLabel);
    infoLabel.setColour(Label::textColourId, Colours::black);
	infoLabel.setBounds(10, 200, 400, 25);
    infoLabel.setLookAndFeel(&menuLF);

    // useful UI debugging components
    addAndMakeVisible(&debugButton);
	debugButton.addListener(this);
	debugButton.setBackgroundColours(Colours::blue, Colours::black);
	debugButton.setBounds(50, 300, 50, 25);
    
    addAndMakeVisible(&loadFilesBtn);
	loadFilesBtn.addListener(this);
	loadFilesBtn.setBounds(50, 350, 70, 25);

 //   addAndMakeVisible(&testBtn);
	//testBtn.addListener(this);
	//testBtn.setBounds(50, 190, 70, 25);

    setUpRecordResampleUI();

    buildSampleStripControls();

    masterGainSlider.addListener(this);
    buildSliders();




    // Preset associated stuff
    addAndMakeVisible(&addPresetBtn);
    addPresetBtn.addListener(this);
    addPresetBtn.setBounds(130, 350, 70, 25);

    addAndMakeVisible(&toggleSetlistBtn);
    toggleSetlistBtn.addListener(this);
    toggleSetlistBtn.setBounds(210, 350, 75, 25);

    addChildComponent(&presetPanel);
    presetPanel.setBounds(presetPanelBounds);


    // Settings associated stuff
    addAndMakeVisible(&toggleSettingsBtn);
    toggleSettingsBtn.addListener(this);
    toggleSettingsBtn.setBounds(210, 230, 75, 25);
    toggleSettingsBtn.setColour(TextButton::buttonColourId, Colours::black);

    addChildComponent(&settingsPanel);
    settingsPanel.setBounds(settingsPanelBounds);

    formatManager.registerBasicFormats();

    // start timer to update play positions, slider values etc.
    startTimer(50);

        // This tells the GUI to use a custom "theme"
    LookAndFeel::setDefaultLookAndFeel(&myLookAndFeel);
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
    int sliderWidth = (int)(270 / (float) (numChannels + 1));

    // Set master volume first
    addAndMakeVisible(&masterGainSlider);
    masterGainSlider.setSliderStyle(Slider::LinearVertical);
    masterGainSlider.setRange(0.0, 1.0, 0.01);
    masterGainSlider.setValue(0.0, false);
    masterGainSlider.setBounds(PAD_AMOUNT, PAD_AMOUNT, sliderWidth, waveformControlHeight);
    masterGainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);
    masterGainSlider.setColour(Slider::thumbColourId, Colours::darkgrey);
    masterGainSlider.setColour(Slider::backgroundColourId, (Colours::darkgrey).darker());
    masterGainSlider.setLookAndFeel(&menuLF);

    // then individual channel volumes
    for(int i = 0; i < numChannels; ++i)
    {
        slidersArray.add(new Slider("channel " + String(i) + " vol"));
        addAndMakeVisible(slidersArray[i]);
        slidersArray[i]->setSliderStyle(Slider::LinearVertical);
        slidersArray[i]->addListener(this);
        slidersArray[i]->setRange(0.0, 1.0, 0.01);
        slidersArray[i]->setBounds(PAD_AMOUNT + (i + 1) * sliderWidth, PAD_AMOUNT, sliderWidth - 1, waveformControlHeight);
        slidersArray[i]->setValue(getProcessor()->getChannelGain(i));

        Colour sliderColour = getProcessor()->getChannelColour(i);
        slidersArray[i]->setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);
        slidersArray[i]->setLookAndFeel(&menuLF);
        slidersArray[i]->setColour(Slider::thumbColourId, sliderColour);
        slidersArray[i]->setColour(Slider::backgroundColourId, sliderColour.darker());
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

    recordBtn.setPercentDone(ourProcessor->getRecordingPrecountPercent(),
                             ourProcessor->getRecordingPercent());
    resampleBtn.setPercentDone(ourProcessor->getResamplingPrecountPercent(),
                               ourProcessor->getResamplingPercent());

    // see if the host has changed the master gain
    masterGainSlider.setValue(ourProcessor->getParameter(mlrVSTAudioProcessor::pMasterGainParam));

    // update the playback position     
    for(int i = 0; i < sampleStripControlArray.size(); ++i)
    {
        sampleStripControlArray[i]->updatePlaybackStatus();
        sampleStripControlArray[i]->updateParamsIfChanged();
    }
}

// This is the callback for when the user drags a slider.
void mlrVSTAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &masterGainSlider)
    {
        // It's vital to use setParameterNotifyingHost to change any parameters that are automatable
        // by the host, rather than just modifying them directly, otherwise the host won't know
        // that they've changed.
        getProcessor()->setParameterNotifyingHost(mlrVSTAudioProcessor::pMasterGainParam,
                                                   (float) masterGainSlider.getValue());
    }
    else if (slider == &bpmSlider)
    {
        double newBPM = bpmSlider.getValue();
        getProcessor()->updateGlobalSetting(mlrVSTAudioProcessor::sCurrentBPM, &newBPM);
    }

    else if (slider == &resampleLengthSldr)
    {
        const int resampleLength = (int) resampleLengthSldr.getValue();
        getProcessor()->updateGlobalSetting(mlrVSTAudioProcessor::sResampleLength, &resampleLength);
    }
    else if (slider == &resamplePrecountSldr)
    {
        const int resamplePrecount = (int) resamplePrecountSldr.getValue();
        getProcessor()->updateGlobalSetting(mlrVSTAudioProcessor::sResamplePrecount, &resamplePrecount);
    }
    else if (slider == &resampleBankSldr)
    {
        const int resampleBank = (int) resampleBankSldr.getValue();
        getProcessor()->updateGlobalSetting(mlrVSTAudioProcessor::sResampleBank, &resampleBank);
    }
    else if (slider == &recordLengthSldr)
    {
        const int recordLength = (int) recordLengthSldr.getValue();
        getProcessor()->updateGlobalSetting(mlrVSTAudioProcessor::sRecordLength, &recordLength);
    }
    else if (slider == &recordPrecountSldr)
    {
        const int recordPrecount = (int) recordPrecountSldr.getValue();
        getProcessor()->updateGlobalSetting(mlrVSTAudioProcessor::sRecordPrecount, &recordPrecount);
    }
    else if (slider == &recordBankSldr)
    {
        const int recordBank = (int) recordBankSldr.getValue();
        getProcessor()->updateGlobalSetting(mlrVSTAudioProcessor::sRecordBank, &recordBank);
    }


    else
    {
        // check the channel volume notifications
        for(int i = 0; i < slidersArray.size(); ++i)
        {
            if (slider == slidersArray[i])
            {
                jassert(i < 8);     // we should not have more than 8 channels
                float newChannelGainValue = (float) slidersArray[i]->getValue();

                // let host know about the new value
                // channel0GainParam is the first channel id, so +i to access the rest
                getProcessor()->setChannelGain(i, newChannelGainValue);
            }
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

    else if(btn == &resampleBtn)
    {
        getProcessor()->startResampling();
    }

    else if(btn == &recordBtn)
    {
        getProcessor()->startRecording();
    }

    // USEFUL FOR TESTING
	else if(btn == &loadFilesBtn)
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

    case mlrVSTAudioProcessor::sNumChannels :
        {
            numChannels = *static_cast<const int*>(newValue);
            buildSliders();
            // let the SampleStrips add the right number of buttons
            for(int i = 0; i < sampleStripControlArray.size(); ++i)
                sampleStripControlArray[i]->setNumChannels(numChannels);
        }
    }
}

const void* mlrVSTAudioProcessorEditor::getGlobalSetting(const int &parameterID) const
{ 
    return getProcessor()->getGlobalSetting(parameterID);
}

void mlrVSTAudioProcessorEditor::setUpRecordResampleUI()
{
    addAndMakeVisible(&resampleBtn);
	resampleBtn.addListener(this);
	resampleBtn.setBounds(PAD_AMOUNT, 450, 70, 25);
    resampleBtn.setFont(fontSilk, fontSize);

    addAndMakeVisible(&precountLbl);
    precountLbl.setBounds(70 + 2*PAD_AMOUNT, 430, 50, 20);
    precountLbl.setFont(fontSize);

    addAndMakeVisible(&resamplePrecountSldr);
    resamplePrecountSldr.setBounds(70 + 2*PAD_AMOUNT, 450, 50, 25);
    resamplePrecountSldr.setRange(0.0, 8.0, 1.0);
    resamplePrecountSldr.setLookAndFeel(&menuLF);
    resamplePrecountSldr.setSliderStyle(Slider::LinearBar);
    resamplePrecountSldr.addListener(this);
    const int resamplePrecount = *static_cast<const int*>(getProcessor()->getGlobalSetting(mlrVSTAudioProcessor::sResamplePrecount));
    resamplePrecountSldr.setValue(resamplePrecount);
    
    addAndMakeVisible(&recordLengthLbl);
    recordLengthLbl.setBounds(120 + 3*PAD_AMOUNT, 430, 50, 20);
    recordLengthLbl.setFont(fontSize);

    addAndMakeVisible(&resampleLengthSldr);
    resampleLengthSldr.setBounds(120 + 3*PAD_AMOUNT, 450, 50, 25);
    resampleLengthSldr.setRange(1.0, 32.0, 1.0);
    resampleLengthSldr.setLookAndFeel(&menuLF);
    resampleLengthSldr.setSliderStyle(Slider::LinearBar);
    resampleLengthSldr.addListener(this);
    const int resampleLength = *static_cast<const int*>(getProcessor()->getGlobalSetting(mlrVSTAudioProcessor::sResampleLength));
    resampleLengthSldr.setValue(resampleLength);

    addAndMakeVisible(&bankLbl);
    bankLbl.setBounds(170 + 4*PAD_AMOUNT, 430, 50, 20);
    bankLbl.setFont(fontSize);

    addAndMakeVisible(&resampleBankSldr);
    resampleBankSldr.setBounds(170 + 4*PAD_AMOUNT, 450, 50, 25);
    resampleBankSldr.setRange(0.0, 7.0, 1.0);
    resampleBankSldr.setLookAndFeel(&menuLF);
    resampleBankSldr.setSliderStyle(Slider::LinearBar);
    resampleBankSldr.addListener(this);
    const int resampleBank = *static_cast<const int*>(getProcessor()->getGlobalSetting(mlrVSTAudioProcessor::sResampleBank));
    resampleBankSldr.setValue(resampleBank);



    addAndMakeVisible(&recordBtn);
	recordBtn.addListener(this);
    recordBtn.setBounds(PAD_AMOUNT, 480, 70, 25);
    recordBtn.setFont(fontSilk, fontSize);

    addAndMakeVisible(&recordPrecountSldr);
    recordPrecountSldr.setBounds(70 + 2*PAD_AMOUNT, 480, 50, 25);
    recordPrecountSldr.setRange(0.0, 8.0, 1.0);
    recordPrecountSldr.setLookAndFeel(&menuLF);
    recordPrecountSldr.setSliderStyle(Slider::LinearBar);
    recordPrecountSldr.addListener(this);
    const int recordPrecount = *static_cast<const int*>(getProcessor()->getGlobalSetting(mlrVSTAudioProcessor::sRecordPrecount));
    recordPrecountSldr.setValue(recordPrecount);

    addAndMakeVisible(&recordLengthSldr);
    recordLengthSldr.setBounds(120 + 3*PAD_AMOUNT, 480, 50, 25);
    recordLengthSldr.setRange(1.0, 32.0, 1.0);
    recordLengthSldr.setLookAndFeel(&menuLF);
    recordLengthSldr.setSliderStyle(Slider::LinearBar);
    recordLengthSldr.addListener(this);
    const int recordLength = *static_cast<const int*>(getProcessor()->getGlobalSetting(mlrVSTAudioProcessor::sRecordLength));
    recordLengthSldr.setValue(recordLength);

    addAndMakeVisible(&recordBankSldr);
    recordBankSldr.setBounds(170 + 4*PAD_AMOUNT, 480, 50, 25);
    recordBankSldr.setRange(0.0, 7.0, 1.0);
    recordBankSldr.setLookAndFeel(&menuLF);
    recordBankSldr.setSliderStyle(Slider::LinearBar);
    recordBankSldr.addListener(this);
    const int recordBank = *static_cast<const int*>(getProcessor()->getGlobalSetting(mlrVSTAudioProcessor::sRecordBank));
    resampleBankSldr.setValue(recordBank);

}

