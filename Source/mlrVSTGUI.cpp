/*

mlrVSTGUI - Contains the basis for the GUI. Note this is initially called
once then destroyed as part of the initialisation process by
the host.

This class loads strips for the samples as seperate GUI
components. Panels for settings / setlist etc are also
loaded from here.

*/

#include "PluginProcessor.h"
#include "mlrVSTGUI.h"

//==============================================================================
mlrVSTGUI::mlrVSTGUI (mlrVSTAudioProcessor* owner, const int &newNumChannels, const int &newNumStrips)
    : AudioProcessorEditor (owner),

    // Communication ////////////////////////
    parent(owner),
    // Style / positioning objects //////////
    myLookAndFeel(), overrideLF(),

    // Fonts ///////////////////////////////////////////////////////
    fontSize(12.f), defaultFont("ProggyCleanTT", 18.f, Font::plain),

    // Volume controls //////////////////////////////////////////////////
    masterGainSlider("master gain"), masterSliderLabel("master", "mstr"),
    slidersArray(), slidersMuteBtnArray(),

    // Tempo controls ///////////////////////////////////////
    bpmSlider("bpm slider"), bpmLabel(),
    quantiseSettingsCbox("quantise settings"),
    quantiseLabel("quantise label", "quant."),

    // Buttons //////////////////////////////////
    loadFilesBtn("load samples", "load samples"),
    sampleStripToggle("sample strip toggle", DrawableButton::ImageRaw),
    patternStripToggle("pattern strip toggle", DrawableButton::ImageRaw),
    sampleImg(), patternImg(),

    // Record / Resample / Pattern UI ////////////////////////////////////////
    precountLbl("precount", "precount"), recordLengthLbl("length", "length"), bankLbl("bank", "bank"),
    recordPrecountSldr(), recordLengthSldr(), recordBankSldr(),
    resamplePrecountSldr(), resampleLengthSldr(), resampleBankSldr(),
    patternPrecountSldr(), patternLengthSldr(), patternBankSldr(),
    recordBtn("record", Colours::black, Colours::white),
    resampleBtn("resample", Colours::black, Colours::white),
    patternBtn("pattern", Colours::black, Colours::white),

    // branding
    vstNameLbl("vst label", "mlrVST"),

    // Misc ///////////////////////////////////////////
    lastDisplayedPosition(),
    debugButton("loadfile", DrawableButton::ImageRaw),    // temporary button

    // Presets //////////////////////////////////////////////////
    presetLabel("presets", "presets"), presetCbox(),
    presetPrevBtn("prev", 0.25, Colours::black, Colours::white),
    presetNextBtn("next", 0.75, Colours::black, Colours::white),
    addPresetBtn("add", "add preset"),
    toggleSetlistBtn("setlist"),
    presetPanelBounds(294, PAD_AMOUNT, THUMBNAIL_WIDTH, 725),
    presetPanel(presetPanelBounds, owner),


    // Settings ///////////////////////////////////////
    numChannels(newNumChannels), useExternalTempo(true),
    toggleSettingsBtn("settings"),
    settingsPanelBounds(294, PAD_AMOUNT, THUMBNAIL_WIDTH, 725),
    settingsPanel(settingsPanelBounds, owner, this),

    // Mappings //////////////////////////////////////
    mappingPanelBounds(294, PAD_AMOUNT, THUMBNAIL_WIDTH, 725),
    toggleMappingBtn("mappings"),
    mappingPanel(mappingPanelBounds, owner),

    // SampleStrip controls ///////////////////////////
    sampleStripControlArray(), numStrips(newNumStrips),
    waveformControlHeight( (GUI_HEIGHT - numStrips * PAD_AMOUNT) / numStrips),
    waveformControlWidth(THUMBNAIL_WIDTH),

    // Overlays //////////////////////////////
    patternStripArray(), hintOverlay(owner)
{
    DBG("GUI loaded " << " strips of height " << waveformControlHeight);

    parent->addChangeListener(this);

    // these are compile time constants
    setSize(GUI_WIDTH, GUI_HEIGHT);
    setWantsKeyboardFocus(true);

    int xPosition = PAD_AMOUNT;
    int yPosition = PAD_AMOUNT;

    // set up volume sliders
    buildSliders(xPosition, yPosition);

    // add the bpm slider and quantise settings
    setupTempoUI(xPosition, yPosition);

    // set up the various panels (settings, mapping, etc)
    // and the buttons that control them
    setupPanels(xPosition, yPosition);

    // preset associated UI elements
    setupPresetUI(xPosition, yPosition);


    // useful UI debugging components
    addAndMakeVisible(&debugButton);
    debugButton.addListener(this);
    debugButton.setColour(DrawableButton::backgroundColourId, Colours::blue);
    debugButton.setBounds(50, 300, 50, 25);

    sampleImg.setImage(ImageCache::getFromMemory(BinaryData::waveform_png, BinaryData::waveform_pngSize));
    patternImg.setImage(ImageCache::getFromMemory(BinaryData::pattern_png, BinaryData::pattern_pngSize));

    addAndMakeVisible(&sampleStripToggle);
    sampleStripToggle.addListener(this);
    sampleStripToggle.setImages(&sampleImg);
    sampleStripToggle.setBounds(100, 300, 70, 25);
    displayMode = modeSampleStrips;

    addAndMakeVisible(&loadFilesBtn);
    loadFilesBtn.addListener(this);
    loadFilesBtn.setBounds(50, 350, 100, 25);

    setUpRecordResampleUI();
    buildSampleStripControls(numStrips);
    setupPatternOverlays();

    masterGainSlider.addListener(this);



    // start timer to update play positions, slider values etc.
    startTimer(50);

    addAndMakeVisible(&vstNameLbl);
    vstNameLbl.setBounds(PAD_AMOUNT, 600, 250, 50);
    vstNameLbl.setFont(Font("ProggyCleanTT", 40.f, Font::plain));
    vstNameLbl.setColour(Label::textColourId, Colours::white);

    addChildComponent(&hintOverlay);
    const int overlayHeight = 150;
    hintOverlay.setBounds(0, GUI_HEIGHT/2 - overlayHeight/2, GUI_WIDTH, overlayHeight);

    // This tells the GUI to use a custom "theme"
    LookAndFeel::setDefaultLookAndFeel(&myLookAndFeel);
}

mlrVSTGUI::~mlrVSTGUI()
{
    parent->removeChangeListener(this);
    sampleStripControlArray.clear(true);

    DBG("GUI destructor finished.");
}

void mlrVSTGUI::buildSampleStripControls(const int &newNumStrips)
{
    // make sure we start from scratch
    sampleStripControlArray.clear(true);

    // rebuild the actual audio objects
    parent->setGlobalSetting(GlobalSettings::sNumSampleStrips, &newNumStrips);

    // what size should the strips be
    waveformControlHeight = ( (GUI_HEIGHT - newNumStrips * PAD_AMOUNT) / newNumStrips);

    // Add SampleStripControls, loading settings from the corresponding SampleStrip
    for(int i = 0; i < newNumStrips; ++i)
    {
        // this is passed to the SampleStripControl to allow data to be stored
        SampleStrip * currentStrip = parent->getSampleStrip(i);

        sampleStripControlArray.add(new SampleStripControl(i, waveformControlWidth,
            waveformControlHeight, numChannels, currentStrip, parent));

        int stripX = getWidth() - waveformControlWidth - PAD_AMOUNT;
        int stripY = PAD_AMOUNT + i * (waveformControlHeight + PAD_AMOUNT);
        sampleStripControlArray[i]->setBounds(stripX, stripY, waveformControlWidth, waveformControlHeight);
        addAndMakeVisible( sampleStripControlArray[i] );
        sampleStripControlArray[i]->toBack();

        // Programmatically load parameters
        for(int p = SampleStrip::FirstParam; p < SampleStrip::NumGUIParams; ++p)
        {
            const void *newValue = parent->getSampleStripParameter(p, i);
            sampleStripControlArray[i]->recallParam(p, newValue, true);
        }
        DBG("params loaded for strip #" << i);
    }

    numStrips = newNumStrips;
    DBG("SampleStripControl array built, size " << numStrips);
}

void mlrVSTGUI::setupPatternOverlays()
{
    // TODO: not hard coded!
    int numPatterns = 4;
    int overlayHeight = ( (GUI_HEIGHT - numPatterns * PAD_AMOUNT) / numPatterns);

    for (int i = 0; i < numPatterns; ++i)
    {
        PatternRecording * currentRecording = parent->getPatternRecording(i);
        patternStripArray.add(new PatternStripControl(i, parent, currentRecording,
            waveformControlWidth, overlayHeight));

        // add the overlay (but don't make visible yet)
        addChildComponent(patternStripArray[i]);

        int stripX = getWidth() - waveformControlWidth - PAD_AMOUNT;
        int stripY = PAD_AMOUNT + i * (overlayHeight + PAD_AMOUNT);
        patternStripArray[i]->setBounds(stripX, stripY, waveformControlWidth, overlayHeight);
    }
}



//==============================================================================
void mlrVSTGUI::paint (Graphics& g)
{
    g.fillAll(Colours::grey);      // fill with background colour
}

void mlrVSTGUI::changeListenerCallback(ChangeBroadcaster *)
{
    DBG("CHANGE");

    if (useExternalTempo)
    {
        AudioPlayHead::CurrentPositionInfo newPos(parent->lastPosInfo);
        if (lastDisplayedPosition != newPos) bpmSlider.setValue(newPos.bpm);
    }
    else
    {
        const double currentBPM = *static_cast<const double*>(parent->getGlobalSetting(GlobalSettings::sCurrentBPM));
        bpmSlider.setValue(currentBPM);
    }

    const int newNumChannels = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sNumChannels));
    if (newNumChannels != numChannels)
    {
        numChannels = newNumChannels;
        int xPosition = PAD_AMOUNT;
        int yPosition = PAD_AMOUNT;

        buildSliders(xPosition, yPosition);

        // let the SampleStrips add the right number of buttons
        for(int i = 0; i < sampleStripControlArray.size(); ++i)
            sampleStripControlArray[i]->setNumChannels(numChannels);
    }

    const int recordLength = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sRecordLength));
    const int recordPrecount = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sRecordPrecount));
    const int recordBank = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sRecordBank));
    recordLengthSldr.setValue(recordLength, NotificationType::dontSendNotification);
    recordPrecountSldr.setValue(recordPrecount, NotificationType::dontSendNotification);
    recordBankSldr.setValue(recordBank, NotificationType::dontSendNotification);

    const int resampleLength = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sResampleLength));
    const int resamplePrecount = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sResamplePrecount));
    const int resampleBank = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sResampleBank));
    resampleLengthSldr.setValue(resampleLength, NotificationType::dontSendNotification);
    resamplePrecountSldr.setValue(resamplePrecount, NotificationType::dontSendNotification);
    resampleBankSldr.setValue(resampleBank, NotificationType::dontSendNotification);

    const int patternLength = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sPatternLength));
    const int patternPrecount = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sPatternPrecount));
    const int patternBank = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sPatternBank));
    patternLengthSldr.setValue(patternLength, NotificationType::dontSendNotification);
    patternPrecountSldr.setValue(patternPrecount, NotificationType::dontSendNotification);
    patternBankSldr.setValue(patternBank, NotificationType::dontSendNotification);

    // see if the host has changed the master gain
    const float masterGain = *static_cast<const float*>(parent->getGlobalSetting(GlobalSettings::sMasterGain));

    masterGainSlider.setValue(masterGain, NotificationType::dontSendNotification);
    for(int c = 0; c < numChannels; ++c)
        slidersArray[c]->setValue(parent->getChannelGain(c), NotificationType::dontSendNotification);

}

//==============================================================================
// This timer periodically checks whether any of the filter's
// parameters have changed. In pratical terms, this is usually
// to see if the host has modified the parameters.
void mlrVSTGUI::timerCallback()
{
    if (parent->areWeRecording())
        recordBtn.setPercentDone(parent->getRecordingPrecountPercent(),
        parent->getRecordingPercent());
    else recordBtn.setPercentDone(0.0, 0.0);

    if (parent->areWeResampling())
        resampleBtn.setPercentDone(parent->getResamplingPrecountPercent(),
        parent->getResamplingPercent());
    else resampleBtn.setPercentDone(0.0, 0.0);

    if (parent->isPatternRecording())
        patternBtn.setPercentDone(parent->getPatternPrecountPercent(),
        parent->getPatternPercent());
    else patternBtn.setPercentDone(0.0, 0.0);

    // see if the modifier button status has changed
    const int modifierStatus = parent->getModifierBtnState();

    if (modifierStatus != MappingEngine::rmNoBtn)
        hintOverlay.setVisible(true);
    else
        hintOverlay.setVisible(false);

    if (displayMode == modeSampleStrips)
    {
        // update the playback position
        for(int i = 0; i < sampleStripControlArray.size(); ++i)
        {
            sampleStripControlArray[i]->updatePlaybackStatus();
            sampleStripControlArray[i]->updateParamsIfChanged();
        }
    }
    else if(displayMode == modePatternStrips)
    {
        for(int i = 0; i < patternStripArray.size(); ++i)
        {
            patternStripArray[i]->repaint();
        }
    }
}

// This is the callback for when the user drags a slider.
void mlrVSTGUI::sliderValueChanged(Slider* slider)
{
    if (slider == &masterGainSlider)
    {
        const float newMasterGain = (float) masterGainSlider.getValue();
        parent->setGlobalSetting(GlobalSettings::sMasterGain, &newMasterGain);
    }
    else if (slider == &bpmSlider)
    {
        double newBPM = bpmSlider.getValue();
        parent->setGlobalSetting(GlobalSettings::sCurrentBPM, &newBPM);
    }

    // resampling sliders /////////////////
    else if (slider == &resampleLengthSldr)
    {
        const int resampleLength = (int) resampleLengthSldr.getValue();
        parent->setGlobalSetting(GlobalSettings::sResampleLength, &resampleLength);
    }
    else if (slider == &resamplePrecountSldr)
    {
        const int resamplePrecount = (int) resamplePrecountSldr.getValue();
        parent->setGlobalSetting(GlobalSettings::sResamplePrecount, &resamplePrecount);
    }
    else if (slider == &resampleBankSldr)
    {
        const int resampleBank = (int) resampleBankSldr.getValue();
        parent->setGlobalSetting(GlobalSettings::sResampleBank, &resampleBank);
    }

    // recording sliders ////////////////
    else if (slider == &recordLengthSldr)
    {
        const int recordLength = (int) recordLengthSldr.getValue();
        parent->setGlobalSetting(GlobalSettings::sRecordLength, &recordLength);
    }
    else if (slider == &recordPrecountSldr)
    {
        const int recordPrecount = (int) recordPrecountSldr.getValue();
        parent->setGlobalSetting(GlobalSettings::sRecordPrecount, &recordPrecount);
    }
    else if (slider == &recordBankSldr)
    {
        const int recordBank = (int) recordBankSldr.getValue();
        parent->setGlobalSetting(GlobalSettings::sRecordBank, &recordBank);
    }

    // pattern recorder sliders /////////
    else if (slider == &patternLengthSldr)
    {
        const int patternLength = (int) patternLengthSldr.getValue();
        parent->setGlobalSetting(GlobalSettings::sPatternLength, &patternLength);
    }
    else if (slider == &patternPrecountSldr)
    {
        const int patternPrecount = (int) patternPrecountSldr.getValue();
        parent->setGlobalSetting(GlobalSettings::sPatternPrecount, &patternPrecount);
    }
    else if (slider == &patternBankSldr)
    {
        const int patternBank = (int) patternBankSldr.getValue();
        parent->setGlobalSetting(GlobalSettings::sPatternBank, &patternBank);

        // load the precount lengths etc associated with this bank
        const int patternLength = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sPatternLength));
        patternLengthSldr.setValue(patternLength, NotificationType::dontSendNotification);

        const int patternPrecountLength = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sPatternPrecount));
        patternPrecountSldr.setValue(patternPrecountLength, NotificationType::dontSendNotification);
    }

    else
    {
        // check the channel volume notifications
        for(int i = 0; i < slidersArray.size(); ++i)
        {
            if (slider == slidersArray[i])
            {
                jassert(i < 8);     // we should not have more than 8 channels
                const float newChannelGainValue = (float) slidersArray[i]->getValue();

                // let host know about the new value
                // channel0GainParam is the first channel id, so +i to access the rest
                parent->setChannelGain(i, newChannelGainValue);
            }
        }
    }
}

void mlrVSTGUI::comboBoxChanged(ComboBox* comboBoxChanged)
{
    if (comboBoxChanged == &quantiseSettingsCbox)
    {
        const int choice = quantiseSettingsCbox.getSelectedId();

        if (choice > 0)
            parent->setGlobalSetting(GlobalSettings::sQuantiseMenuSelection, &choice);
    }
}

void mlrVSTGUI::buttonClicked(Button* btn)
{
    if(btn == &toggleSetlistBtn)
    {
        const bool currentlyVisible = presetPanel.isVisible();
        // close any existing panels
        closePanels();
        // Toggle the visibility of the setlist manager panel
        presetPanel.setVisible(!currentlyVisible);
        // and update the button
        toggleSetlistBtn.setToggleState(!currentlyVisible, NotificationType::dontSendNotification);
    }

    else if(btn == &toggleSettingsBtn)
    {
        const bool currentlyVisible = settingsPanel.isVisible();
        // close any existing panels
        closePanels();
        // Toggle the visibility of the settings panel
        settingsPanel.setVisible(!currentlyVisible);
        // and update the button
        toggleSettingsBtn.setToggleState(!currentlyVisible, NotificationType::dontSendNotification);
    }

    else if(btn == &toggleMappingBtn)
    {
        const bool currentlyVisible = mappingPanel.isVisible();
        // close any existing panels
        closePanels();
        // Toggle the settings panel
        mappingPanel.setVisible(!currentlyVisible);
        // and update the button
        toggleMappingBtn.setToggleState(!currentlyVisible, NotificationType::dontSendNotification);
    }

    else if(btn == &addPresetBtn)
    {
#if JUCE_MODAL_LOOPS_PERMITTED

        AlertWindow w("save preset", "please enter a name for this preset", AlertWindow::NoIcon);

        w.setColour(AlertWindow::textColourId, Colours::black);
        w.setColour(AlertWindow::outlineColourId, Colours::black);

        w.addTextEditor("text", "preset name", "preset name:");

        w.addButton("ok", 1, KeyPress (KeyPress::returnKey, 0, 0));
        w.addButton("cancel", 0, KeyPress (KeyPress::escapeKey, 0, 0));

        if (w.runModalLoop() != 0) // is they picked 'ok'
        {
            // this is the text they entered..
            String newPresetName(w.getTextEditorContents("text"));

            parent->addPreset(newPresetName);
        }

        // update the preset list to show new additions
        presetPanel.refreshPresetLists();
#endif
    }

    else if(btn == &resampleBtn)
    {
        parent->startResampling();
    }

    else if(btn == &recordBtn)
    {
        parent->startRecording();
    }

    else if(btn == &patternBtn)
    {
        parent->startPatternRecording();
    }

    // load files manually using file dialog
    else if(btn == &loadFilesBtn)
    {
        FileChooser myChooser ("Please choose a file:",
            File::getSpecialLocation(File::userDesktopDirectory),
            parent->getWildcardFormats());

        // ask user to load at least one file
        if(myChooser.browseForMultipleFilesToOpen())
        {
            // if sucessful, try to add these to the sample pool
            Array<File> newFiles = myChooser.getResults();
            for (int i = 0; i < newFiles.size(); ++i)
            {
                File currentFile = newFiles[i];
                parent->addNewSample(currentFile);
            }
        }
    }

    else if (btn == &sampleStripToggle)
    {
        if (displayMode == modeSampleStrips)
        {
            sampleStripToggle.setImages(&patternImg);
            displayMode = modePatternStrips;

            for(int i = 0; i < patternStripArray.size(); ++i)
                patternStripArray[i]->setVisible(true);
            for(int i = 0; i < sampleStripControlArray.size(); ++i)
                sampleStripControlArray[i]->setVisible(false);
        }
        else if (displayMode == modePatternStrips)
        {
            sampleStripToggle.setImages(&sampleImg);
            displayMode = modeSampleStrips;

            for(int i = 0; i < patternStripArray.size(); ++i)
                patternStripArray[i]->setVisible(false);
            for(int i = 0; i < sampleStripControlArray.size(); ++i)
                sampleStripControlArray[i]->setVisible(true);
        }
    }

    else if (btn == &debugButton)
    {
        parent->addPreset("test");
        DBG("Hai");
    }

    else
    {
        for (int s = 0; s < slidersMuteBtnArray.size(); ++s)
        {
            if (btn == slidersMuteBtnArray[s])
                parent->setChannelMute(s, btn->getToggleState());
        }
    }
}

// Setting handling stuff
void mlrVSTGUI::setGlobalSetting(const int &parameterID, const void *newValue)
{
    /* First let the processor store the setting (as mlrVSTGUI
    will lose these on closing. We don't need to update listeners as the GUI
    is the source of the change (at that is the only object notified anyway).
    */
    parent->setGlobalSetting(parameterID, newValue, false);

    // a few settings are of interest to the GUI
    switch (parameterID)
    {
    case GlobalSettings::sUseExternalTempo :
        {
            useExternalTempo = *static_cast<const bool*>(newValue);

            if (useExternalTempo)
            {
                bpmSlider.setEnabled(false);
                bpmLabel.setText("bpm (external)", NotificationType::dontSendNotification);
            }
            else
            {
                bpmSlider.setEnabled(true);
                bpmLabel.setText("bpm (internal)", NotificationType::dontSendNotification);
            }

            break;
        }

    case GlobalSettings::sNumChannels :
        {
            numChannels = *static_cast<const int*>(newValue);

            int xPosition = PAD_AMOUNT; int yPosition = PAD_AMOUNT;
            buildSliders(xPosition, yPosition);
            // let the SampleStrips add the right number of buttons
            for(int i = 0; i < sampleStripControlArray.size(); ++i)
                sampleStripControlArray[i]->setNumChannels(numChannels);
        }
    }
}

void mlrVSTGUI::setUpRecordResampleUI()
{
    const int buttonWidth = 70;
    const int buttonHeight = 25;

    const int sliderWidth = 50;
    const int sliderHeight = buttonHeight;

    int xPosition = 90;
    int yPosition = 430;

    // Labels //////////////////////
    addAndMakeVisible(&precountLbl);
    precountLbl.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    precountLbl.setFont(defaultFont);
    xPosition += PAD_AMOUNT + sliderWidth;

    addAndMakeVisible(&recordLengthLbl);
    recordLengthLbl.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    recordLengthLbl.setFont(defaultFont);
    xPosition += PAD_AMOUNT + sliderWidth;

    addAndMakeVisible(&bankLbl);
    bankLbl.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    bankLbl.setFont(defaultFont);
    xPosition = PAD_AMOUNT;
    yPosition += sliderHeight;

    // Resampling sliders / buttons ////////////////////
    addAndMakeVisible(&resampleBtn);
    resampleBtn.addListener(this);
    resampleBtn.setBounds(xPosition, yPosition, buttonWidth, buttonHeight);
    resampleBtn.setFont(defaultFont, fontSize);
    xPosition += PAD_AMOUNT + buttonWidth;

    addAndMakeVisible(&resamplePrecountSldr);
    resamplePrecountSldr.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    resamplePrecountSldr.setRange(0.0, 8.0, 1.0);
    resamplePrecountSldr.setSliderStyle(Slider::LinearBar);
    resamplePrecountSldr.addListener(this);
    const int resamplePrecount = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sResamplePrecount));
    resamplePrecountSldr.setValue(resamplePrecount);
    resamplePrecountSldr.setLookAndFeel(&overrideLF);
    xPosition += sliderWidth + PAD_AMOUNT;

    addAndMakeVisible(&resampleLengthSldr);
    resampleLengthSldr.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    resampleLengthSldr.setRange(1.0, 32.0, 1.0);
    resampleLengthSldr.setSliderStyle(Slider::LinearBar);
    resampleLengthSldr.addListener(this);
    const int resampleLength = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sResampleLength));
    resampleLengthSldr.setValue(resampleLength);
    resampleLengthSldr.setLookAndFeel(&overrideLF);
    xPosition += sliderWidth + PAD_AMOUNT;

    addAndMakeVisible(&resampleBankSldr);
    resampleBankSldr.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    resampleBankSldr.setRange(0.0, 7.0, 1.0);
    resampleBankSldr.setSliderStyle(Slider::LinearBar);
    resampleBankSldr.addListener(this);
    const int resampleBank = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sResampleBank));
    resampleBankSldr.setValue(resampleBank);
    resampleBankSldr.setLookAndFeel(&overrideLF);

    xPosition = PAD_AMOUNT;
    yPosition += sliderHeight + PAD_AMOUNT;

    // Recording sliders / buttons ////////////////////
    addAndMakeVisible(&recordBtn);
    recordBtn.addListener(this);
    recordBtn.setBounds(xPosition, yPosition, buttonWidth, buttonHeight);
    recordBtn.setFont(defaultFont, fontSize);
    xPosition += PAD_AMOUNT + buttonWidth;

    addAndMakeVisible(&recordPrecountSldr);
    recordPrecountSldr.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    recordPrecountSldr.setRange(0.0, 8.0, 1.0);
    recordPrecountSldr.setSliderStyle(Slider::LinearBar);
    recordPrecountSldr.addListener(this);
    const int recordPrecount = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sRecordPrecount));
    recordPrecountSldr.setValue(recordPrecount);
    recordPrecountSldr.setLookAndFeel(&overrideLF);
    xPosition += sliderWidth + PAD_AMOUNT;

    addAndMakeVisible(&recordLengthSldr);
    recordLengthSldr.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    recordLengthSldr.setRange(1.0, 32.0, 1.0);
    recordLengthSldr.setSliderStyle(Slider::LinearBar);
    recordLengthSldr.addListener(this);
    const int recordLength = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sRecordLength));
    recordLengthSldr.setValue(recordLength);
    recordLengthSldr.setLookAndFeel(&overrideLF);
    xPosition += sliderWidth + PAD_AMOUNT;

    addAndMakeVisible(&recordBankSldr);
    recordBankSldr.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    recordBankSldr.setRange(0.0, 7.0, 1.0);
    recordBankSldr.setSliderStyle(Slider::LinearBar);
    recordBankSldr.addListener(this);
    const int recordBank = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sRecordBank));
    resampleBankSldr.setValue(recordBank);
    resampleBankSldr.setLookAndFeel(&overrideLF);
    xPosition = PAD_AMOUNT;
    yPosition += sliderHeight + PAD_AMOUNT;

    // Pattern recording sliders / buttons ////////////////////
    addAndMakeVisible(&patternBtn);
    patternBtn.addListener(this);
    patternBtn.setBounds(xPosition, yPosition, buttonWidth, buttonHeight);
    patternBtn.setFont(defaultFont, fontSize);
    xPosition += PAD_AMOUNT + buttonWidth;

    addAndMakeVisible(&patternPrecountSldr);
    patternPrecountSldr.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    patternPrecountSldr.setRange(0.0, 8.0, 1.0);
    patternPrecountSldr.setSliderStyle(Slider::LinearBar);
    patternPrecountSldr.addListener(this);
    const int patternPrecount = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sPatternPrecount));
    patternPrecountSldr.setValue(patternPrecount);
    patternPrecountSldr.setLookAndFeel(&overrideLF);
    xPosition += sliderWidth + PAD_AMOUNT;

    addAndMakeVisible(&patternLengthSldr);
    patternLengthSldr.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    patternLengthSldr.setRange(1.0, 32.0, 1.0);
    patternLengthSldr.setSliderStyle(Slider::LinearBar);
    patternLengthSldr.addListener(this);
    const int patternLength = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sPatternLength));
    patternLengthSldr.setValue(patternLength);
    patternLengthSldr.setLookAndFeel(&overrideLF);
    xPosition += sliderWidth + PAD_AMOUNT;

    addAndMakeVisible(&patternBankSldr);
    patternBankSldr.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    patternBankSldr.setRange(0.0, 7.0, 1.0);
    patternBankSldr.setSliderStyle(Slider::LinearBar);
    patternBankSldr.addListener(this);
    const int patternBank = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sPatternBank));
    patternBankSldr.setValue(patternBank);
    patternBankSldr.setLookAndFeel(&overrideLF);
}

void mlrVSTGUI::buildSliders(int &xPosition, int &yPosition)
{
    // work out the correct height width
    const int sliderWidth = (int)(270 / (float) (numChannels + 1));
    const int sliderHeight = 66;    // TODO: no magic numbers!

    // clear any existing sliders
    slidersArray.clear();
    slidersMuteBtnArray.clear();

    // Set master volume first
    addAndMakeVisible(&masterGainSlider);
    masterGainSlider.setSliderStyle(Slider::LinearVertical);

    masterGainSlider.setRange(0.0, 1.0, 0.01);
    masterGainSlider.setValue(0.0, NotificationType::dontSendNotification);
    masterGainSlider.setBounds(xPosition, yPosition, sliderWidth, sliderHeight);
    masterGainSlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    masterGainSlider.setColour(Slider::thumbColourId, Colours::darkgrey);
    masterGainSlider.setColour(Slider::backgroundColourId, (Colours::darkgrey).darker());

    // and its label
    addAndMakeVisible(&masterSliderLabel);
    masterSliderLabel.setBounds(xPosition, yPosition + sliderHeight, sliderWidth, 16);
    masterSliderLabel.setColour(Label::backgroundColourId, Colours::black);
    masterSliderLabel.setColour(Label::textColourId, Colours::white);
    masterSliderLabel.setFont(defaultFont);

    xPosition += 1;

    // then individual channel volumes
    for(int i = 0; i < numChannels; ++i)
    {
        xPosition += sliderWidth;

        slidersArray.add(new Slider("channel " + String(i) + " vol"));
        addAndMakeVisible(slidersArray[i]);
        slidersArray[i]->setSliderStyle(Slider::LinearVertical);
        slidersArray[i]->addListener(this);
        slidersArray[i]->setRange(0.0, 1.0, 0.01);
        slidersArray[i]->setBounds(xPosition, yPosition, sliderWidth - 1, sliderHeight);
        slidersArray[i]->setValue(parent->getChannelGain(i));

        Colour sliderColour = parent->getChannelColour(i);
        slidersArray[i]->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
        slidersArray[i]->setColour(Slider::thumbColourId, sliderColour);
        slidersArray[i]->setColour(Slider::backgroundColourId, sliderColour.darker());

        // add the labels
        slidersMuteBtnArray.add(new ToggleButton("ch" + String(i)));
        addAndMakeVisible(slidersMuteBtnArray[i]);
        slidersMuteBtnArray[i]->setBounds(xPosition, yPosition + sliderHeight, sliderWidth - 1, 16);
        slidersMuteBtnArray[i]->setColour(Label::backgroundColourId, Colours::black);
        slidersMuteBtnArray[i]->setColour(Label::textColourId, Colours::white);
        slidersMuteBtnArray[i]->addListener(this);
        slidersMuteBtnArray[i]->setToggleState(parent->getChannelMuteStatus(i),
                                               NotificationType::dontSendNotification);
    }

    // increment yPosition for the next GUI item
    xPosition = PAD_AMOUNT;
    yPosition += sliderHeight + 3 * PAD_AMOUNT;
}

void mlrVSTGUI::setupTempoUI(int &xPosition, int &yPosition)
{
    int bpmSliderWidth = 180, bpmSliderHeight = 30;
    int bpmLabelWidth = bpmSliderWidth, bpmLabelHeight = 16;

    int quantiseBoxWidth = 80, quantiseBoxHeight = 30;
    int quantiseLabelWidth = quantiseBoxWidth, quantiseLabelHeight = 16;

    ///////////////////////
    // First add the labels
    addAndMakeVisible(&bpmLabel);
    bpmLabel.setBounds(xPosition, yPosition, bpmLabelWidth, bpmLabelHeight);
    bpmLabel.setColour(Label::textColourId, Colours::white);
    bpmLabel.setColour(Label::backgroundColourId, Colours::black);
    bpmLabel.setFont(defaultFont);
    xPosition += bpmLabelWidth + PAD_AMOUNT;

    addAndMakeVisible(&quantiseLabel);
    quantiseLabel.setBounds(xPosition, yPosition, quantiseLabelWidth, quantiseLabelHeight);
    quantiseLabel.setColour(Label::textColourId, Colours::white);
    quantiseLabel.setColour(Label::backgroundColourId, Colours::black);
    quantiseLabel.setFont(defaultFont);

    xPosition = PAD_AMOUNT;
    yPosition += quantiseLabelHeight;

    ///////////////////////////////
    // Then add sliders and boxes
    addAndMakeVisible(&bpmSlider);
    bpmSlider.setBounds(xPosition, yPosition, bpmSliderWidth, bpmSliderHeight);
    bpmSlider.setColour(Slider::backgroundColourId, Colours::black.withAlpha(0.3f));
    bpmSlider.setSliderStyle(Slider::LinearBar);
    bpmSlider.setRange(20.0, 300.0, 0.01);
    bpmSlider.setTextBoxIsEditable(true);
    bpmSlider.addListener(this);
    bpmSlider.setLookAndFeel(&overrideLF);

    useExternalTempo = *static_cast<const bool*>(getGlobalSetting(GlobalSettings::sUseExternalTempo));
    if (useExternalTempo)
    {
        bpmSlider.setEnabled(false);
        bpmLabel.setText("bpm (external)", NotificationType::dontSendNotification);
    }
    else
    {
        double newBPM = *static_cast<const double*>(getGlobalSetting(GlobalSettings::sCurrentBPM));
        bpmSlider.setValue(newBPM);
        bpmLabel.setText("bpm (internal)", NotificationType::dontSendNotification);
    }

    xPosition += bpmSliderWidth + PAD_AMOUNT;

    // add dropdown box for quantisation settings
    addAndMakeVisible(&quantiseSettingsCbox);
    quantiseSettingsCbox.setBounds(xPosition, yPosition, quantiseBoxWidth, quantiseBoxHeight);
    quantiseSettingsCbox.addListener(this);

    // add items to it
    quantiseSettingsCbox.addItem("None", 1);
    for (int i = 2, denom = 1; i < 8; ++i, denom *= 2)
        quantiseSettingsCbox.addItem(String(denom) + "n", i);

    // and load the stored selection if suitable
    const int menuSelection = *static_cast<const int*>(parent->getGlobalSetting(GlobalSettings::sQuantiseMenuSelection));
    if (menuSelection >= 0 && menuSelection < 8) quantiseSettingsCbox.setSelectedId(menuSelection);
    else quantiseSettingsCbox.setSelectedId(1);

    quantiseSettingsCbox.setLookAndFeel(&overrideLF);

    xPosition = PAD_AMOUNT;
    yPosition += bpmSliderHeight + PAD_AMOUNT;
}

void mlrVSTGUI::setupPanels(int &xPosition, int &yPosition)
{
    const int buttonHeight = 25;
    const int buttonWidth = 66;

    // button to toggle the settings panel
    addAndMakeVisible(&toggleSettingsBtn);
    toggleSettingsBtn.addListener(this);
    toggleSettingsBtn.setBounds(xPosition, yPosition, buttonWidth, buttonHeight);
    // add the actual panel
    addChildComponent(&settingsPanel);
    settingsPanel.setBounds(settingsPanelBounds);
    xPosition += PAD_AMOUNT + buttonWidth;

    // button to toggle the setlist panel
    addAndMakeVisible(&toggleSetlistBtn);
    toggleSetlistBtn.addListener(this);
    toggleSetlistBtn.setBounds(xPosition, yPosition, buttonWidth, buttonHeight);
    // add the actual button
    addChildComponent(&presetPanel);
    presetPanel.setBounds(presetPanelBounds);
    xPosition += PAD_AMOUNT + buttonWidth;

    // button to toggle mappings panel
    addAndMakeVisible(&toggleMappingBtn);
    toggleMappingBtn.addListener(this);
    toggleMappingBtn.setBounds(xPosition, yPosition, buttonWidth, buttonHeight);
    // add the actual panel
    addChildComponent(&mappingPanel);
    mappingPanel.setBounds(mappingPanelBounds);

    xPosition = PAD_AMOUNT;
    yPosition += buttonHeight + PAD_AMOUNT;
}

void mlrVSTGUI::setupPresetUI(int &xPosition, int &yPosition)
{
    addAndMakeVisible(&presetLabel);
    presetLabel.setBounds(xPosition, yPosition, 270, 16);
    presetLabel.setColour(Label::textColourId, Colours::white);
    presetLabel.setColour(Label::backgroundColourId, Colours::black);
    presetLabel.setFont(defaultFont);
    yPosition += 16;

    addAndMakeVisible(&presetCbox);
    presetCbox.setBounds(xPosition, yPosition, 180, 25);
    presetCbox.addItem("test1", 1);
    presetCbox.addItem("test2", 2);
    xPosition += 180;


    addAndMakeVisible(&presetPrevBtn);
    presetPrevBtn.setBounds(xPosition, yPosition, 25, 25);
    xPosition += 25;

    addAndMakeVisible(&presetNextBtn);
    presetNextBtn.setBounds(xPosition, yPosition, 25, 25);
    xPosition += 25;

    addAndMakeVisible(&addPresetBtn);
    addPresetBtn.addListener(this);
    addPresetBtn.setBounds(xPosition, yPosition, 40, 25);

    xPosition = PAD_AMOUNT;
    yPosition += 25 + PAD_AMOUNT;
}


// Force any visible panels to close
void mlrVSTGUI::closePanels()
{
    presetPanel.setVisible(false);
    toggleSetlistBtn.setToggleState(false, NotificationType::dontSendNotification);

    mappingPanel.setVisible(false);
    toggleMappingBtn.setToggleState(false, NotificationType::dontSendNotification);

    settingsPanel.setVisible(false);
    toggleSettingsBtn.setToggleState(false, NotificationType::dontSendNotification);
}