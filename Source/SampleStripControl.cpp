/*
==============================================================================

SampleStripControl.cpp
Created: 6 Sep 2011 12:38:13pm
Author:  Hemmer

Custom component to display a waveform (corresponding to an mlr row)

==============================================================================
*/


#include "PluginEditor.h"
#include <cstdlib>
#include <cmath>

SampleStripControl::SampleStripControl(const int &id, const int &width, const int &height,
                                       const int &newNumChannels,
                                       SampleStrip * const dataStripLink,
                                       mlrVSTAudioProcessor * const owner) :

    // ID / communication //////////////////////////
    processor(owner), sampleStripID(id),
    dataStrip(dataStripLink), stripChanged(true),

    // GUI dimensions //////////////////////////////
    componentHeight(height), componentWidth(width), controlbarSize(16),
    waveformPaintBounds(0, controlbarSize, componentWidth, (componentHeight - controlbarSize)),

    // SampleStrip GUI ////////////////////////////////////////
    menuLF(), fontSize(7.4f), backgroundColour(Colours::black),
    chanLbl("channel label", "CHAN"), channelButtonArray(),
    volLbl("volume label", "VOL"), stripVolumeSldr("volume"),
    modeLbl("mode", "MODE"), selPlayMode("select playmode"), isLatchedBtn("LATCH"),
    playspeedLbl("playspeed label", "PLAY SPEED"), playspeedSldr("playspeed"),
    speedLockBtn("speed lock", DrawableButton::ImageRaw), isReversedBtn("NORM"),
    lockImg(), unlockImg(), times2("x2"), div2("/2"), selNumChunks(),
    trackNumberLbl("track number", String(sampleStripID)), filenameLbl("filename", "No File"),
    popupLocators(),

    // Waveform control ////////////////////////
    visualSelectionStart(0), visualSelectionEnd(componentWidth), visualSelectionLength(componentWidth),
    visualChunkSize(0.0), numChunks(8),
    selectionStartBeforeDrag(0), selectionPointToChange(0), selectionPointFixed(0),
    mouseDownMods(), rightMouseDown(false), selectedHitZone(0),
    thumbnailScaleFactor(1.0), currentSample(0),

    // Settings ///////////////////////
    numChannels(newNumChannels),
    isSpeedLocked(false), isLatched(true),
    isReversed(false), isPlaying(false),
    playbackPercentage(0.0f)

{
    // load binary data for lock icon
    lockImg.setImage(ImageCache::getFromMemory(BinaryData::locked_png, BinaryData::locked_pngSize));
    unlockImg.setImage(ImageCache::getFromMemory(BinaryData::unlocked_png, BinaryData::unlocked_pngSize));

    // does the heavy UI positioning   
    buildUI();
    buildNumBlocksList(8);

    popupLocators.add(new Label("none"));
    addAndMakeVisible(popupLocators.getLast());
    popupLocators.getLast()->setBounds(0, controlbarSize, 1, 1);
    popupLocators.add(new Label("samples"));
    addAndMakeVisible(popupLocators.getLast());
    popupLocators.getLast()->setBounds(1 * componentWidth / 4, controlbarSize, 1, 1);
    popupLocators.add(new Label("recordings"));
    addAndMakeVisible(popupLocators.getLast());
    popupLocators.getLast()->setBounds(2 * componentWidth / 4, controlbarSize, 1, 1);
    popupLocators.add(new Label("resamplings"));
    addAndMakeVisible(popupLocators.getLast());
    popupLocators.getLast()->setBounds(3 * componentWidth / 4, controlbarSize, 1, 1);

    dataStrip->addChangeListener(this);

    // listen for user input
    selPlayMode.addListener(this);
    playspeedSldr.addListener(this);
    stripVolumeSldr.addListener(this);
    isReversedBtn.addListener(this);
    isLatchedBtn.addListener(this);
    selNumChunks.addListener(this);
    times2.addListener(this);
    div2.addListener(this);
    speedLockBtn.addListener(this);
}

SampleStripControl::~SampleStripControl()
{
    dataStrip->removeChangeListener(this);
    channelButtonArray.clear(true);
    popupLocators.clear(true);
}

void SampleStripControl::buildNumBlocksList(const int &newMaxNumBlocks)
{
    selNumChunks.clear();
    for (int i = 1; i <= newMaxNumBlocks; ++i)
        selNumChunks.addItem(String(i), i);
}

void SampleStripControl::changeListenerCallback(ChangeBroadcaster * sender)
{
    /* TODO: this gets called A LOT during playback (as playback percentage
       is considered a parameter). Maybe seperate that out from the param setup?
    */
    if (sender == dataStrip)
    {
        stripChanged = true;
    }
}

void SampleStripControl::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{

    if (comboBoxThatHasChanged == &selNumChunks)
    {
        numChunks = selNumChunks.getSelectedId();
        visualChunkSize = (visualSelectionLength / (float) numChunks);

        // It's actually quite complicated/risky to change this on the fly,
        // it's easiest just stop the sample first instead!
        dataStrip->stopSamplePlaying();
        dataStrip->setSampleStripParam(SampleStrip::pNumChunks,
                                       &numChunks);
        repaint();
    }
    else if (comboBoxThatHasChanged == &selPlayMode)
    {
        // -1 is because ComboBox reserves 0 for when menu is closed without choosing
        int newPlayMode = selPlayMode.getSelectedId() - 1;
        dataStrip->setSampleStripParam(SampleStrip::pPlayMode, &newPlayMode);
    }

}

void SampleStripControl::buttonClicked(Button *btn)
{
    // See if any of the channel selection buttons were chosen
    for (int i = 0; i < channelButtonArray.size(); ++i)
    {
        if (channelButtonArray[i] == btn)
        {
            // update the processor / GUI
            updateChannelColours(i);
            // ...so we can let the processor know
            processor->switchChannels(i, sampleStripID);
            // no need to check the rest of the buttons
            return;
        }
    }

    if (btn == &isLatchedBtn)
    {
        isLatched = !isLatched;
        dataStrip->setSampleStripParam(SampleStrip::pIsLatched, &isLatched);
    }
    else if (btn == &isReversedBtn)
    {
        isReversed = !isReversed;
        dataStrip->setSampleStripParam(SampleStrip::pIsReversed, &isReversed);
        String newButtonText = (isReversed) ? "REV" : "NORM";
        isReversedBtn.setButtonText(newButtonText);
    }
    else if (btn == &times2)
    {
        processor->modPlaySpeed(2.0, sampleStripID);
    }
    else if (btn == &div2)
    {
        processor->modPlaySpeed(0.5, sampleStripID);
    }
    else if (btn == &speedLockBtn)
    {
        bool newIsSpeedLocked = !isSpeedLocked;
        dataStrip->setSampleStripParam(SampleStrip::pIsPlaySpeedLocked,
                                       &newIsSpeedLocked);
    }

}

void SampleStripControl::sliderValueChanged(Slider *sldr)
{
    if (sldr == &stripVolumeSldr)
    {
        float newStripVol = (float)(stripVolumeSldr.getValue());
        thumbnailScaleFactor = newStripVol;
        dataStrip->setSampleStripParam(SampleStrip::pStripVolume, &newStripVol);
    }
    else if(sldr == &playspeedSldr)
    {
        double newPlaySpeed = playspeedSldr.getValue();
        dataStrip->setSampleStripParam(SampleStrip::pPlaySpeed, &newPlaySpeed);
    }
}

void SampleStripControl::updateChannelColours(const int &newChannel)
{

    backgroundColour = channelButtonArray[newChannel]->getBackgroundColour();

    isLatchedBtn.setColour(ToggleButton::textColourId, backgroundColour);
    isReversedBtn.setColour(ToggleButton::textColourId, backgroundColour);

    stripVolumeSldr.setColour(Slider::thumbColourId, backgroundColour);
    stripVolumeSldr.setColour(Slider::backgroundColourId, backgroundColour.darker());

    playspeedSldr.setColour(Slider::thumbColourId, backgroundColour);
    playspeedSldr.setColour(Slider::backgroundColourId, backgroundColour.darker());

    isLatchedBtn.setColour(ToggleButton::textColourId, backgroundColour);
    isReversedBtn.setColour(ToggleButton::textColourId, backgroundColour);

    // TODO: background colour of TextButton not controlled when mousedown
    times2.setColour(TextButton::textColourOffId, backgroundColour);
    times2.setColour(TextButton::buttonOnColourId, backgroundColour);

    div2.setColour(TextButton::textColourOffId, backgroundColour);
    div2.setColour(TextButton::buttonOnColourId, backgroundColour);

    speedLockBtn.setBackgroundColours(backgroundColour, backgroundColour);

    repaint();
}

// This is particuarly usful if the number of channels changes
void SampleStripControl::buildUI()
{

    int newXposition = 0;

    // This is the track number
    addAndMakeVisible(&trackNumberLbl);
    trackNumberLbl.setBounds(0, componentHeight - controlbarSize, controlbarSize, controlbarSize);
    trackNumberLbl.setColour(Label::backgroundColourId, Colours::black);
    trackNumberLbl.setColour(Label::textColourId, Colours::white);
    trackNumberLbl.setFont(fontSize);
    
    addAndMakeVisible(&filenameLbl);
    filenameLbl.setColour(Label::backgroundColourId, Colours::white.withAlpha(0.5f));
    filenameLbl.setColour(Label::textColourId, Colours::black);
    filenameLbl.setJustificationType(Justification::right);
    filenameLbl.setBounds(controlbarSize, componentHeight - controlbarSize, 200, controlbarSize);
    filenameLbl.setFont(fontSize);


    addAndMakeVisible(&chanLbl);
    chanLbl.setBounds(0, 0, 35, controlbarSize);
    chanLbl.setColour(Label::backgroundColourId, Colours::black);
    chanLbl.setColour(Label::textColourId, Colours::white);
    chanLbl.setFont(fontSize);

    // clear existing buttons
    channelButtonArray.clear();

    for(int chan = 0; chan < numChannels; ++chan)
    {
        channelButtonArray.add(new DrawableButton("button" + String(chan), DrawableButton::ImageRaw));
        addAndMakeVisible(channelButtonArray.getLast());
        channelButtonArray.getLast()->setBounds(35 + chan * controlbarSize, 0, controlbarSize, controlbarSize);
        channelButtonArray.getLast()->addListener(this);
        Colour chanColour = processor->getChannelColour(chan);
        channelButtonArray.getLast()->setBackgroundColours(chanColour, chanColour);
    }

    int previousChannel = *static_cast<const int*>(dataStrip->getSampleStripParam(SampleStrip::pCurrentChannel));
    if (previousChannel >= numChannels){
        DBG("Current channel outside range: resetting to channel 0.");
        updateChannelColours(0);
        processor->switchChannels(0, sampleStripID);
    }

    newXposition = 35 + (numChannels) * controlbarSize;

    addAndMakeVisible(&volLbl);
    volLbl.setBounds(newXposition, 0, 28, controlbarSize);
    volLbl.setColour(Label::backgroundColourId, Colours::black);
    volLbl.setColour(Label::textColourId, Colours::white);
    volLbl.setFont(fontSize);

    newXposition += 28;

    addAndMakeVisible(&stripVolumeSldr);
    stripVolumeSldr.setSliderStyle(Slider::LinearBar);
    stripVolumeSldr.setColour(Slider::textBoxTextColourId, Colours::white);
    stripVolumeSldr.setBounds(newXposition, 0, 60, controlbarSize);
    stripVolumeSldr.setRange(0.0, 2.0, 0.01);
    stripVolumeSldr.setTextBoxIsEditable(true);
    stripVolumeSldr.setLookAndFeel(&menuLF);

    newXposition += 60;

    addAndMakeVisible(&modeLbl);
    modeLbl.setBounds(newXposition, 0, 33, controlbarSize);
    modeLbl.setColour(Label::backgroundColourId, Colours::black);
    modeLbl.setColour(Label::textColourId, Colours::white);
    modeLbl.setFont(fontSize);

    newXposition += 33;

    addAndMakeVisible(&selPlayMode);
    selPlayMode.clear();
    for (int i = 0; i < SampleStrip::NUM_PLAY_MODES; ++i)
    {
        selPlayMode.addItem(dataStrip->getPlayModeName(i), i+1);
    }
    selPlayMode.setBounds(newXposition, 0, 86, controlbarSize);
    selPlayMode.setLookAndFeel(&menuLF);

    newXposition += 86;

    addAndMakeVisible(&isLatchedBtn);
    isLatchedBtn.setBounds(newXposition, 0, 40, controlbarSize);
    isLatchedBtn.setColour(ToggleButton::textColourId, backgroundColour);

    newXposition += 40;

    addAndMakeVisible(&playspeedLbl);
    playspeedLbl.setBounds(newXposition, 0, 64, controlbarSize);
    playspeedLbl.setColour(Label::backgroundColourId, Colours::black);
    playspeedLbl.setColour(Label::textColourId, Colours::white);
    playspeedLbl.setFont(fontSize);

    newXposition += 64;

    addAndMakeVisible(&playspeedSldr);
    playspeedSldr.setSliderStyle(Slider::LinearBar);
    playspeedSldr.setColour(Slider::textBoxTextColourId, Colours::white);
    playspeedSldr.setBounds(newXposition, 0, 80, controlbarSize);
    playspeedSldr.setRange(0.0, 4.0, 0.001);
    playspeedSldr.setTextBoxIsEditable(true);
    playspeedSldr.setLookAndFeel(&menuLF);
    

    newXposition += 80;
    addAndMakeVisible(&speedLockBtn);
    speedLockBtn.setImages(&unlockImg);
    speedLockBtn.setBounds(newXposition, 0, 16, 16);

    newXposition += 16;
    addAndMakeVisible(&isReversedBtn);
    isReversedBtn.setBounds(newXposition, 0, 35, controlbarSize);

    newXposition += 35;
    addAndMakeVisible(&div2);
    div2.setBounds(newXposition, 0, 20, controlbarSize);

    newXposition += 20;
    addAndMakeVisible(&times2);
    times2.setBounds(newXposition, 0, 20, controlbarSize);

    newXposition += 20;
    addAndMakeVisible(&selNumChunks);
    selNumChunks.setBounds(newXposition, 0, 32, controlbarSize);
    selNumChunks.setLookAndFeel(&menuLF);
}

void SampleStripControl::mouseDown(const MouseEvent &e)
{
    // Save modifers of the initial press
    mouseDownMods = e.mods;

    // right click displays hit zones for sample popup menus
    if (e.mods == ModifierKeys::rightButtonModifier && e.y > controlbarSize)
    {
        rightMouseDown = true;
        selectedHitZone = e.x / (componentWidth / 4);
        repaint();        
    }

    // middle click-drag to move the selection
    else if (e.mods == ModifierKeys::middleButtonModifier)
    {
        selectionStartBeforeDrag = visualSelectionStart;
    }

    // double click to select whole waveform
    else if (e.mods == ModifierKeys::leftButtonModifier && e.getNumberOfClicks() == 2)
    {
        // select whole sample by default
        visualSelectionStart = 0;
        visualSelectionEnd = componentWidth;
        visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
        visualChunkSize = (visualSelectionLength / (float) numChunks);

        // update the selection
        dataStrip->setSampleStripParam(SampleStrip::pVisualStart, &visualSelectionStart);
        dataStrip->setSampleStripParam(SampleStrip::pVisualEnd, &visualSelectionEnd);

        processor->calcInitialPlaySpeed(sampleStripID);

        // repaint when we next get a timed update
        stripChanged = true;
    }

    // Determine which point we are moving based on proximity to either
    // start or end points. The use of pointers here means we don't need
    // to worry about which end of the selection is actually moving.
    if ( abs(e.x - visualSelectionStart) > abs(e.x - visualSelectionEnd) )
    {
        selectionPointToChange = &visualSelectionEnd;
        selectionPointFixed = &visualSelectionStart;
    }
    else
    {
        selectionPointToChange = &visualSelectionStart;
        selectionPointFixed = &visualSelectionEnd;
    }
}

void SampleStripControl::mouseUp(const MouseEvent& e)
{
    // TODO: middle click to delete sample under cursor in menu?
    if (e.mods == ModifierKeys::rightButtonModifier)
    {
        switch (selectedHitZone)
        {
        // user selected NONE
        case pNone :
            {
                dataStrip->stopSamplePlaying();
                const AudioSample *newSample = 0;
                dataStrip->setSampleStripParam(SampleStrip::pAudioSample, newSample);
                break;
            }

        // user selected SAMPLE
        case pSample :
            {
                PopupMenu sampleMenu = PopupMenu();
                const int currentSamplePoolSize = processor->getSamplePoolSize(mlrVSTAudioProcessor::pSamplePool);

                // for each sample, populate the menu
                for (int i = 0; i < currentSamplePoolSize; ++i)
                {
                    // get the sample name
                    String iFileName = processor->getSampleName(i, mlrVSTAudioProcessor::pSamplePool);
                    // +1 because 0 is result for no item clicked
                    sampleMenu.addItem(i + 1, iFileName);

                    DBG("option none selected");
                }

                // show the menu and store choice
                int fileChoice = sampleMenu.showMenu(PopupMenu::Options().withTargetComponent(popupLocators[selectedHitZone]));
                // subtract 1 from loop above
                fileChoice -= 1;

                // if we have a valid choice, pick it
                if (fileChoice >= 0 && fileChoice < currentSamplePoolSize)
                {
                    DBG("sample option: " << fileChoice);
                    selectNewSample(fileChoice, mlrVSTAudioProcessor::pSamplePool);
                }
                break;
            }

        case pRecord :
            {
                PopupMenu recordPoolMenu = PopupMenu();
                const int currentRecordPoolSize = processor->getSamplePoolSize(mlrVSTAudioProcessor::pRecordPool);

                // populate menu
                for (int m = 0; m < currentRecordPoolSize; ++m)
                {
                    // +1 because 0 is result for no item clicked
                    recordPoolMenu.addItem(m + 1, "record " + String(m));
                }

                // show the menu and store choice
                int fileChoice = recordPoolMenu.showMenu(PopupMenu::Options().withTargetComponent(popupLocators[selectedHitZone]));
                // subtract 1 from loop above
                fileChoice -= 1;

                // if we have a valid choice, pick it
                if (fileChoice >= 0 && fileChoice < currentRecordPoolSize)
                {
                    DBG("record option: " << fileChoice);
                    selectNewSample(fileChoice, mlrVSTAudioProcessor::pRecordPool);

                }
                break;
            }

        case pResample :
            {
                PopupMenu resamplePoolMenu = PopupMenu();
                const int currentResamplePoolSize = processor->getSamplePoolSize(mlrVSTAudioProcessor::pResamplePool);

                // populate menu
                for (int m = 0; m < currentResamplePoolSize; ++m)
                {
                    // +1 because 0 is result for no item clicked
                    resamplePoolMenu.addItem(m + 1, "resample " + String(m));
                }

                // show the menu and store choice
                int fileChoice = resamplePoolMenu.showMenu(PopupMenu::Options().withTargetComponent(popupLocators[selectedHitZone]));
                // subtract 1 from loop above
                fileChoice -= 1;

                // if we have a valid choice, pick it
                if (fileChoice >= 0 && fileChoice < currentResamplePoolSize)
                {
                    DBG("resample option: " << fileChoice);
                    selectNewSample(fileChoice, mlrVSTAudioProcessor::pResamplePool);
                }
                break;
            }
        }

        // stop drawing hit zones
        rightMouseDown = false;
        repaint();
    }
}

void SampleStripControl::mouseDrag(const MouseEvent &e)
{
    // while RMB is held, see which hit zone it selects
    if (e.mods == ModifierKeys::rightButtonModifier)
    {
        selectedHitZone = e.x / (componentWidth / 4);
    }

    // CASE: ctrl-shift-LMB allows us to snap to specific intervals
    else if ((e.mods == (ModifierKeys::ctrlModifier +
                         ModifierKeys::leftButtonModifier +
                         ModifierKeys::shiftModifier)))
    {
        // TODO: have snapping interval as an option
        int eighth = componentWidth / 16;

        // round to the nearest snap point
        int newSeg = (int) floor(0.5 + e.x / (float) eighth);
        
        // update the changing part of the selection to the snapped position
        *selectionPointToChange = (int)(newSeg * eighth);
    }

    // CASE: traditional drag-to-select
    else if (mouseDownMods == ModifierKeys::leftButtonModifier)
    {
        int mouseX = e.x;

        // Make sure we don't select outside the waveform
        if (mouseX > componentWidth) mouseX = componentWidth;
        if (mouseX < 0) mouseX = 0;

        *selectionPointFixed = e.getMouseDownX();
        *selectionPointToChange = mouseX;
    }

    // CASE: moving the entire selection (fixed size)
    else if (mouseDownMods == ModifierKeys::middleButtonModifier)
    {
        // Don't select outside the component!
        int newStart = selectionStartBeforeDrag + e.getDistanceFromDragStartX();
        int newEnd = newStart + visualSelectionLength;

        if(newStart < 0)
        {
            newStart = 0;
            newEnd = visualSelectionLength;
        }
        else if (newEnd > componentWidth)
        {
            newEnd = componentWidth;
            newStart = newEnd - visualSelectionLength;
        }

        visualSelectionStart = newStart;
        visualSelectionEnd = newEnd;
    }

    // CASE: just moving one end of an existing selection
    else if (mouseDownMods == (ModifierKeys::ctrlModifier + ModifierKeys::leftButtonModifier))
    {
        // Don't select outside the component!
        int newValue = e.x;
        if (newValue < 0) newValue = 0;
        else if (newValue > componentWidth) newValue = componentWidth;

        *selectionPointToChange = newValue;
    }


    // Swap selection positions if inverse selection is made
    if (visualSelectionEnd < visualSelectionStart)
    {
        int temp = visualSelectionStart;
        visualSelectionStart = visualSelectionEnd;
        visualSelectionEnd = temp;

        // same for the pointers
        int *tempPointer = selectionPointFixed;
        selectionPointFixed = selectionPointToChange;
        selectionPointToChange = tempPointer;
    }

    visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
    visualChunkSize = (visualSelectionLength / (float) numChunks);

    // send the new selection to the SampleStrips
    dataStrip->setSampleStripParam(SampleStrip::pVisualStart, &visualSelectionStart);
    dataStrip->setSampleStripParam(SampleStrip::pVisualEnd, &visualSelectionEnd);

    // update the play speed to account for new selection,
    // of course this doesn't change if we are just moving the 
    // selection (i.e. using the middle mouse button)
    if (mouseDownMods != ModifierKeys::middleButtonModifier)
        processor->calcPlaySpeedForSelectionChange(sampleStripID);

    // repaint when we next get a timed update
    stripChanged = true;
}

void SampleStripControl::paint(Graphics& g)
{
    // Start with the background colour
    g.setColour(backgroundColour);
    g.fillRect(waveformPaintBounds);

    // Draw the current sample waveform in white
    g.setColour(Colours::white.withAlpha(0.75f));

    if(currentSample)
    {
        currentSample->drawChannels(g, waveformPaintBounds,
                                    (float) thumbnailScaleFactor);
    }

    // Indicate where we are in playback
    if (isPlaying)
    {
        g.setColour(Colours::black.withAlpha(0.25f));
        int visualPlaybackPoint = (int)(playbackPercentage * visualSelectionLength);
        
        if (!isReversed)
        {
            g.fillRect(visualSelectionStart, controlbarSize,
                visualPlaybackPoint, componentHeight - controlbarSize);
        }
        else
        {
            g.fillRect(visualSelectionStart + visualPlaybackPoint, controlbarSize,
                visualSelectionLength - visualPlaybackPoint, componentHeight - controlbarSize);
        }
    }


    /* Finally we grey out the parts of the sample which aren't in
       the selection and paint stripes to indicate what button 
       will do what.
    */
    g.setColour(Colours::black.withAlpha(0.5f));
    g.fillRect(0, controlbarSize, visualSelectionStart, componentHeight - controlbarSize); 
    g.fillRect(visualSelectionEnd, controlbarSize, componentWidth - visualSelectionEnd, componentHeight - controlbarSize); 

    /* Add alternate strips to make it clear which
    button plays which part of the sample. */
    g.setColour(Colours::black.withAlpha(0.15f));
    for(int i = 1; i < numChunks; i+=2)
    {
        g.fillRect((float) (visualSelectionStart + i * visualChunkSize),
            (float) controlbarSize, visualChunkSize, (float)(componentHeight - controlbarSize));
    }

    // draw hit zones
    if (rightMouseDown)
    {
        g.setColour(Colours::black.withAlpha(0.2f));
        g.fillRect(waveformPaintBounds);
        g.setColour(Colours::black.withAlpha(0.6f));
        g.fillRect(selectedHitZone * componentWidth / 4, controlbarSize, componentWidth / 4, waveformPaintBounds.getHeight());

        g.setColour(Colours::white);
        g.drawFittedText("NONE", 0 * componentWidth / 4, componentHeight / 2, componentWidth / 4, 20, Justification::horizontallyCentred, 1);
        g.drawFittedText("SAMPLES", 1 * componentWidth / 4, componentHeight / 2, componentWidth / 4, 20, Justification::horizontallyCentred, 1);
        g.drawFittedText("RECORDINGS", 2 * componentWidth / 4, componentHeight / 2, componentWidth / 4, 20, Justification::horizontallyCentred, 1);
        g.drawFittedText("RESAMPLINGS", 3 * componentWidth / 4, componentHeight / 2, componentWidth / 4, 20, Justification::horizontallyCentred, 1);
    }
}

void SampleStripControl::filesDropped(const StringArray& files, int /*x*/, int /*y*/)
{
    // assume that all files fail (pessimistic I know)
    int fileIndex = -1;

    // try to add each of the loaded files to the sample pool
    for (int i = 0; i < files.size(); ++i)
    {
        File currentSampleFile(files[i]);
        DBG("Dragged file: " << files[i]);

        // if a file is sucessfully loaded, update the file index
        fileIndex = processor->addNewSample(currentSampleFile);
    }

    // If we have a legitimate file
    if (fileIndex >= 0)
    {
        selectNewSample(fileIndex, mlrVSTAudioProcessor::pSamplePool);
    }
}

void SampleStripControl::selectNewSample(const int &fileChoice, const int &poolID)
{
    // and let the audio processor update the sample strip
    const AudioSample * newSample = processor->getAudioSample(fileChoice, poolID);
    dataStrip->setSampleStripParam(SampleStrip::pAudioSample, newSample);

    // select whole sample by default
    visualSelectionStart = 0;
    visualSelectionEnd = componentWidth;
    visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
    visualChunkSize = (visualSelectionLength / (float) numChunks);

    // update the selection
    dataStrip->setSampleStripParam(SampleStrip::pVisualStart, &visualSelectionStart);
    dataStrip->setSampleStripParam(SampleStrip::pVisualEnd, &visualSelectionEnd);

    // and try to find the best playback speed (i.e. closest to 1).
    processor->calcInitialPlaySpeed(sampleStripID);

    // repaint when we next get a timed update
    stripChanged = true;
}

void SampleStripControl::recallParam(const int &paramID, const void *newValue, const bool &doRepaint)
{
    switch (paramID)
    {
    case SampleStrip::pCurrentChannel :
        {
            const int newCurrentChannel = *static_cast<const int*>(newValue);
            updateChannelColours(newCurrentChannel);
            break;
        }

    case SampleStrip::pNumChunks :
        {
            numChunks = *static_cast<const int*>(newValue);
            selNumChunks.setSelectedId(numChunks, true);
            visualChunkSize = (float)(visualSelectionLength / (float) numChunks);
            break;
        }

    case SampleStrip::pPlayMode :
        {
            const int newPlayMode = *static_cast<const int*>(newValue);
            selPlayMode.setSelectedId(newPlayMode + 1);
            break;
        }

    case SampleStrip::pIsLatched :
        {
            isLatched = *static_cast<const bool*>(newValue);
            isLatchedBtn.setToggleState(isLatched, false);
            break;
        }

    case SampleStrip::pIsReversed :
        {
            isReversed = *static_cast<const bool*>(newValue);
            isReversedBtn.setToggleState(isReversed, false);
            String btnText = (isReversed) ? "REV" : "NORM";
            isReversedBtn.setButtonText(btnText);
            break;
        }

    case SampleStrip::pIsPlaying :
        isPlaying = *static_cast<const bool*>(newValue);
        break;

    case SampleStrip::pPlaybackPercentage :
        playbackPercentage = *static_cast<const float*>(newValue);
        break;


    case SampleStrip::pVisualStart :
        {
            visualSelectionStart =  *static_cast<const int*>(newValue);
            visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
            visualChunkSize = (visualSelectionLength / (float) numChunks);
            break;
        }

    case SampleStrip::pVisualEnd :
        {
            visualSelectionEnd =  *static_cast<const int*>(newValue);
            visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
            visualChunkSize = (visualSelectionLength / (float) numChunks);
            break;
        }

    case SampleStrip::pStripVolume :
        {
            float newStripVolume = *static_cast<const float*>(newValue);
            stripVolumeSldr.setValue(newStripVolume, true);
            break;
        }

    case SampleStrip::pPlaySpeed :
        {
            double newPlaySpeed = *static_cast<const double*>(newValue);
            playspeedSldr.setValue(newPlaySpeed, false);
            break;
        }

    case SampleStrip::pIsPlaySpeedLocked :
        {
            bool newIsSpeedLocked = *static_cast<const bool*>(newValue);
            if (newIsSpeedLocked != isSpeedLocked)
            {
                isSpeedLocked = newIsSpeedLocked;
                if (isSpeedLocked)
                {
                    speedLockBtn.setImages(&lockImg);
                    playspeedSldr.setEnabled(false);
                }
                else
                {
                    speedLockBtn.setImages(&unlockImg);
                    playspeedSldr.setEnabled(true);
                }
            }
            break;
        }

    case SampleStrip::pAudioSample :
        {
            const AudioSample *newSample = static_cast<const AudioSample*>(newValue);

            // Only update if we have to as reloading thumbnail is expensive
            if (newSample != currentSample )
            {
                currentSample = newSample;
                // If the new sample exists
                if (currentSample)
                    filenameLbl.setText(currentSample->getSampleName(), false);
                else
                {
                    filenameLbl.setText("No file", false);
                    playspeedSldr.setValue(1.0, true);
                }
            }
            
            break;
        }

    default :
        jassertfalse;
        DBG("Param not found");
    }

    if(doRepaint) repaint();
}
