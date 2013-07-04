/*
==============================================================================

SampleStripControl.cpp
Created: 6 Sep 2011 12:38:13pm
Author:  Hemmer

Custom component to display a waveform (corresponding to an mlr row)

==============================================================================
*/

#include "mlrVSTGUI.h"
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
    componentHeight(height), componentWidth(width), controlbarSize(18),
    maxWaveformHeight(componentHeight - controlbarSize),
    waveformPaintBounds(0, controlbarSize, componentWidth, (componentHeight - controlbarSize)),

    // SampleStrip GUI ////////////////////////////////////////
    overrideLF(), defaultFont("ProggyCleanTT", 18.f, Font::plain),
    backgroundColour(Colours::black),
    chanLbl("channel label", "chan"), channelButtonArray(),
    volLbl("volume label", "vol"), stripVolumeSldr(TextDragSlider::SliderTypeFloat),
    modeLbl("mode", "mode"), selPlayMode("select playmode"), isLatchedBtn("latch"),
    playspeedLbl("playspeed label", "speed"), playspeedSldr(TextDragSlider::SliderTypeFloat),
    speedLockBtn("speed lock", DrawableButton::ImageRaw),
    isReversedBtn("reverse", 0.0f, Colours::black, Colours::white),
    lockImg(), unlockImg(), times2("x2"), div2("/2"), selNumChunks(TextDragSlider::SliderTypeInt),
    trackNumberLbl("track number", String(sampleStripID)), filenameLbl("filename", "no file"),
    popupLocators(),

    // Waveform control ////////////////////////
    visualSelectionStart(0), visualSelectionEnd(componentWidth), visualSelectionLength(componentWidth),
    visualChunkSize(0.0), numChunksLabel("divs", "divs"), numChunks(8),
    selectionStartBeforeDrag(0), selectionPointToChange(0), selectionPointFixed(0),
    mouseDownMods(), rightMouseDown(false), modifierBtnStatus(-1), selectedHitZone(0),
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
    stripVolumeSldr.addChangeListener(this);
    playspeedSldr.addChangeListener(this);
    selNumChunks.addChangeListener(this);

    selPlayMode.addListener(this);
    isReversedBtn.addListener(this);
    isLatchedBtn.addListener(this);
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


void SampleStripControl::changeListenerCallback(ChangeBroadcaster * sender)
{
    /* TODO: this gets called A LOT during playback (as playback percentage
    is considered a parameter). Maybe seperate that out from the param setup?
    */
    if (sender == dataStrip)
    {
        stripChanged = true;


    }
    else if (sender == &stripVolumeSldr)
    {
        float newStripVol = stripVolumeSldr.getValue();
        thumbnailScaleFactor = newStripVol;
        dataStrip->setSampleStripParam(SampleStrip::pStripVolume, &newStripVol);
    }
    else if(sender == &playspeedSldr)
    {
        double newPlaySpeed = playspeedSldr.getValue();
        dataStrip->setSampleStripParam(SampleStrip::pPlaySpeed, &newPlaySpeed);
    }
    else if (sender == &selNumChunks)
    {
        numChunks = selNumChunks.getValue();
        visualChunkSize = (visualSelectionLength / (float) numChunks);

        // It's actually quite complicated/risky to change this on the fly,
        // it's easiest just stop the sample first instead!
        dataStrip->stopSamplePlaying();
        dataStrip->setSampleStripParam(SampleStrip::pNumChunks, &numChunks);
        repaint();
    }
    else
    {
        DBG("ChangeBroadcaster not found!");
    }

}

void SampleStripControl::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{

    if (comboBoxThatHasChanged == &selPlayMode)
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

        const float newArrowDirection = (isReversed) ? 0.5f : 0.0f;
        isReversedBtn.setDirection(newArrowDirection);
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

void SampleStripControl::updateChannelColours(const int &newChannel)
{
    backgroundColour = processor->getChannelColour(newChannel);

    isLatchedBtn.setColour(ToggleButton::textColourId, backgroundColour);
    isReversedBtn.setColour(CustomArrowButton::arrowColourId, backgroundColour);

    stripVolumeSldr.setColour(Slider::thumbColourId, backgroundColour);
    stripVolumeSldr.setColour(Slider::backgroundColourId, backgroundColour.darker());

    playspeedSldr.setColour(Slider::thumbColourId, backgroundColour);
    playspeedSldr.setColour(Slider::backgroundColourId, backgroundColour.darker());


    // TODO: background colour of TextButton not controlled when mousedown
    times2.setColour(TextButton::textColourOffId, backgroundColour);
    times2.setColour(TextButton::buttonOnColourId, backgroundColour);

    div2.setColour(TextButton::textColourOffId, backgroundColour);
    div2.setColour(TextButton::buttonOnColourId, backgroundColour);

    speedLockBtn.setColour(DrawableButton::backgroundColourId, backgroundColour);

    selNumChunks.setColour(Slider::thumbColourId, backgroundColour);
    selNumChunks.setColour(Slider::backgroundColourId, backgroundColour.darker());

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
    trackNumberLbl.setFont(defaultFont);

    addAndMakeVisible(&filenameLbl);
    filenameLbl.setColour(Label::backgroundColourId, Colours::white.withAlpha(0.5f));
    filenameLbl.setColour(Label::textColourId, Colours::black);
    filenameLbl.setJustificationType(Justification::right);
    filenameLbl.setBounds(controlbarSize, componentHeight - controlbarSize, 200, controlbarSize);
    filenameLbl.setFont(defaultFont);

    addAndMakeVisible(&chanLbl);
    chanLbl.setBounds(0, 0, 38, controlbarSize);
    chanLbl.setColour(Label::backgroundColourId, Colours::black);
    chanLbl.setColour(Label::textColourId, Colours::white);
    chanLbl.setFont(defaultFont);

    newXposition = 38;

    // clear existing buttons
    channelButtonArray.clear();

    for(int chan = 0; chan < numChannels; ++chan)
    {
        channelButtonArray.add(new DrawableButton("button" + String(chan), DrawableButton::ImageRaw));
        addAndMakeVisible(channelButtonArray.getLast());
        channelButtonArray.getLast()->setBounds(newXposition + chan * controlbarSize, 0, controlbarSize, controlbarSize);
        channelButtonArray.getLast()->addListener(this);

        Colour chanColour = processor->getChannelColour(chan);
        channelButtonArray.getLast()->setColour(DrawableButton::backgroundColourId, chanColour);
    }

    int previousChannel = *static_cast<const int*>(dataStrip->getSampleStripParam(SampleStrip::pCurrentChannel));
    if (previousChannel >= numChannels){
        DBG("Current channel outside range: resetting to channel 0.");
        updateChannelColours(0);
        processor->switchChannels(0, sampleStripID);
    }

    newXposition += (numChannels) * controlbarSize;

    addAndMakeVisible(&volLbl);
    volLbl.setBounds(newXposition, 0, 30, controlbarSize);
    volLbl.setColour(Label::backgroundColourId, Colours::black);
    volLbl.setColour(Label::textColourId, Colours::white);
    volLbl.setFont(defaultFont);

    newXposition += 30;

    addAndMakeVisible(&stripVolumeSldr);
    stripVolumeSldr.setBounds(newXposition, 0, 60, controlbarSize);
    stripVolumeSldr.setMaxMin(4.0f, 0.0f);
    newXposition += 60;

    addAndMakeVisible(&modeLbl);
    modeLbl.setBounds(newXposition, 0, 36, controlbarSize);
    modeLbl.setColour(Label::backgroundColourId, Colours::black);
    modeLbl.setColour(Label::textColourId, Colours::white);
    modeLbl.setFont(defaultFont);
    newXposition += 36;

    addAndMakeVisible(&selPlayMode);
    selPlayMode.clear();
    for (int i = 0; i < SampleStrip::NUM_PLAY_MODES; ++i)
    {
        selPlayMode.addItem(dataStrip->getPlayModeName(i), i+1);
    }
    selPlayMode.setBounds(newXposition, 0, 100, controlbarSize);
    selPlayMode.setLookAndFeel(&overrideLF);
    newXposition += 100;

    addAndMakeVisible(&isLatchedBtn);
    isLatchedBtn.setBounds(newXposition, 0, 44, controlbarSize);
    isLatchedBtn.setColour(ToggleButton::textColourId, backgroundColour);
    newXposition += 44;

    addAndMakeVisible(&playspeedLbl);
    playspeedLbl.setBounds(newXposition, 0, 44, controlbarSize);
    playspeedLbl.setColour(Label::backgroundColourId, Colours::black);
    playspeedLbl.setColour(Label::textColourId, Colours::white);
    playspeedLbl.setFont(defaultFont);
    newXposition += 44;

    addAndMakeVisible(&playspeedSldr);
    playspeedSldr.setBounds(newXposition, 0, 80, controlbarSize);
    playspeedSldr.setMaxMin(16.0f, -16.0f);
    newXposition += 80;

    addAndMakeVisible(&speedLockBtn);
    speedLockBtn.setImages(&unlockImg);
    speedLockBtn.setBounds(newXposition, 0, controlbarSize, controlbarSize);
    newXposition += controlbarSize;

    addAndMakeVisible(&isReversedBtn);
    isReversedBtn.setBounds(newXposition, 0, controlbarSize, controlbarSize);
    newXposition += controlbarSize;

    addAndMakeVisible(&div2);
    div2.setBounds(newXposition, 0, 20, controlbarSize);
    newXposition += 20;

    addAndMakeVisible(&times2);
    times2.setBounds(newXposition, 0, 20, controlbarSize);
    newXposition += 20;

    addAndMakeVisible(&numChunksLabel);
    numChunksLabel.setBounds(newXposition, 0, 36, controlbarSize);
    numChunksLabel.setColour(Label::backgroundColourId, Colours::black);
    numChunksLabel.setColour(Label::textColourId, Colours::white);
    numChunksLabel.setFont(defaultFont);
    newXposition += 36;

    addAndMakeVisible(&selNumChunks);
    selNumChunks.setBounds(newXposition, 0, 32, controlbarSize);
    selNumChunks.setMaxMin(16, 1);
    selNumChunks.setDefault(8);

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
    int mouseX = e.x;

    // Make sure we don't select outside the waveform
    if (mouseX > componentWidth) mouseX = componentWidth;
    else if (mouseX < 0) mouseX = 0;

    // CASE: traditional drag-to-select region of sample
    if (mouseDownMods == ModifierKeys::leftButtonModifier)
    {
        *selectionPointFixed = e.getMouseDownX();
        *selectionPointToChange = mouseX;
    }

    // CASE: just moving one end of an existing selection
    else if (mouseDownMods == (ModifierKeys::ctrlModifier + ModifierKeys::leftButtonModifier))
    {
        *selectionPointToChange = mouseX;
    }

    // CASE: RMB is held, see which hit zone it selects
    else if (e.mods == ModifierKeys::rightButtonModifier)
    {
        selectedHitZone = mouseX / (componentWidth / 4);
    }

    // CASE: ctrl-shift-LMB allows us to snap to specific intervals
    else if ((e.mods == (ModifierKeys::ctrlModifier +
        ModifierKeys::leftButtonModifier +
        ModifierKeys::shiftModifier)))
    {
        // TODO: have snapping interval as an option
        int eighth = componentWidth / 16;

        // round to the nearest snap point
        int newSeg = (int) floor(0.5 + mouseX / (float) eighth);

        // update the changing part of the selection to the snapped position
        *selectionPointToChange = (int)(newSeg * eighth);
    }

    // CASE: moving the entire selection (whilst retaining size)
    else if (mouseDownMods == ModifierKeys::middleButtonModifier)
    {
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
    g.fillAll(backgroundColour);

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

    // find which modifier button is held (if any)
    const int modifierStatus = processor->getModifierBtnState();

    // draw hit zones
    if (rightMouseDown)
    {
        g.setColour(Colours::black.withAlpha(0.2f));
        g.fillRect(waveformPaintBounds);
        g.setColour(Colours::black.withAlpha(0.6f));
        g.fillRect(selectedHitZone * componentWidth / 4, controlbarSize, componentWidth / 4, waveformPaintBounds.getHeight());

        g.setColour(Colours::white);
        g.setFont(defaultFont);
        g.drawFittedText("none", 0 * componentWidth / 4, componentHeight / 2, componentWidth / 4, 20, Justification::horizontallyCentred, 1);
        g.drawFittedText("samples", 1 * componentWidth / 4, componentHeight / 2, componentWidth / 4, 20, Justification::horizontallyCentred, 1);
        g.drawFittedText("recordings", 2 * componentWidth / 4, componentHeight / 2, componentWidth / 4, 20, Justification::horizontallyCentred, 1);
        g.drawFittedText("resamplings", 3 * componentWidth / 4, componentHeight / 2, componentWidth / 4, 20, Justification::horizontallyCentred, 1);
    }

    // if a modifier button is held, draw an overlay
    else if (modifierBtnStatus != mlrVSTAudioProcessor::rmNoBtn)
    {
        // paint a translucent background
        g.setColour(Colours::black.withAlpha(0.3f));
        g.fillRect(waveformPaintBounds);

        // TODO: this should NOT be hard coded!
        const int numCols = 8;

        const int rowType = modifierStatus;
        const int spacing = componentWidth / numCols;

        g.setColour(Colours::white);
        g.setFont(defaultFont);

        switch (modifierStatus)
        {
        case mlrVSTAudioProcessor::rmNoBtn : break;
        case mlrVSTAudioProcessor::rmNormalRowMappingBtnA :
        case mlrVSTAudioProcessor::rmNormalRowMappingBtnB :
            {
                for (int c = 0; c < numCols; ++c)
                {
                    const int mappingID = processor->getMonomeMapping(rowType, c);
                    const String mappingName = processor->getSampleStripMappingName(mappingID);

                    g.drawFittedText(mappingName, PAD_AMOUNT + c * spacing, controlbarSize, spacing - PAD_AMOUNT, maxWaveformHeight,
                        Justification::centredLeft, 5, 1.0f);
                }
                break;
            }

        case mlrVSTAudioProcessor::rmPatternBtn :
            {
                // Find out if the pattern associated with this channel is
                // playing or recording and paint a progress strip along the
                // middle of the strip based on that
                if (processor->isPatternPlaying(sampleStripID))
                {
                    g.setColour(backgroundColour.withAlpha(0.5f));
                    const float percentDone = processor->getPatternPercent(sampleStripID);
                    g.fillRect(0, componentHeight / 2, int (componentWidth*percentDone), controlbarSize);
                }
                else if (processor->isPatternRecording(sampleStripID))
                {
                    g.setColour(backgroundColour);
                    const float percentDone = processor->getPatternPercent(sampleStripID);
                    g.fillRect(0, componentHeight / 2, int (componentWidth*percentDone), controlbarSize);
                }

                g.setColour(Colours::white);
                for (int c = 0; c < numCols; ++c)
                {
                    const int mappingID = processor->getMonomeMapping(rowType, c);
                    const String mappingName = processor->getPatternStripMappingName(mappingID);

                    g.drawFittedText(mappingName, PAD_AMOUNT + c * spacing, controlbarSize, spacing - PAD_AMOUNT, maxWaveformHeight,
                        Justification::centredLeft, 5, 1.0f);
                }

                break;
            }
        }
    }
}

void SampleStripControl::filesDropped(const StringArray& files, int /*x*/, int /*y*/)
{
    // assume that all files fail (pessimistic I know)
    int fileIndex = -1;

    // don't add files recursively
    const bool useRecursive = false;

    // try to add each of the loaded files to the sample pool
    for (int i = 0; i < files.size(); ++i)
    {
        File currentSampleFile(files[i]);

        // if a folder is dragged, try to add all files in it
        if (currentSampleFile.isDirectory())
        {

            DirectoryIterator di(currentSampleFile, useRecursive, "*", File::findFiles);

            // iterate through the files in this directory
            while(di.next())
            {
                File childFile = di.getFile();
                DBG("Dragged file (in directory): " << childFile.getFileName());

                // and if so try to load the file
                const int newFileIndex = processor->addNewSample( childFile );

                // if we have a sucessful load, we can set this as the
                // SampleStrip's new sample
                if (newFileIndex != -1) fileIndex = newFileIndex;
            }
        }
        else
        {
            DBG("Dragged file: " << files[i]);

            // try to load the file
            const int newFileIndex = processor->addNewSample(currentSampleFile);

            // if we have a sucessful load, we can set this as the
            // SampleStrip's new sample
            if (newFileIndex != -1) fileIndex = newFileIndex;
        }
    }

    // if any of the the files loaded, select the last
    // file to sucessfully load
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

            // update the slider, but avoid sending a change message as
            // this would result in infinite loop!
            selNumChunks.setValue(numChunks, false);
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

            const float newArrowDirection = (isReversed) ? 0.5f : 0.0f;
            isReversedBtn.setDirection(newArrowDirection);
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
            const float newVol = *static_cast<const float*>(newValue);
            thumbnailScaleFactor = newVol;

            var newStripVolume = newVol;
            stripVolumeSldr.setValue(newStripVolume, false);
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
                {
                    filenameLbl.setText(currentSample->getSampleName(), NotificationType::dontSendNotification);
                }
                else
                {
                    filenameLbl.setText("No file", NotificationType::dontSendNotification);
                    playspeedSldr.setValue(1.0);
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