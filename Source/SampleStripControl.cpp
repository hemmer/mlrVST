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

SampleStripControl::SampleStripControl(const int &id,
                                       const int &width,
                                       const int &height,
                                       const int &newNumChannels,
                                       SampleStrip * const dataStripLink,
                                       mlrVSTAudioProcessorEditor * const owner) :
    componentHeight(height), componentWidth(width), sampleStripID(id),
    backgroundColour(Colours::black), controlbarSize(16), fontSize(7.4f),
    waveformPaintBounds(0, controlbarSize, componentWidth, (componentHeight - controlbarSize)),
    thumbnailScaleFactor(1.0),
    trackNumberLbl("track number", String(sampleStripID)),
    filenameLbl("filename", "No File"),
    chanLbl("channel label", "CHAN"), volLbl("volume label", "VOL"),
    modeLbl("mode", "MODE"), playSpeedLbl("play speed label", "PLAY SPEED"),
    numChannels(newNumChannels),
    channelButtonArray(),
    isReversed(false),
    numChunks(8), currentSample(0),
    visualSelectionStart(0), visualSelectionEnd(componentWidth), visualChunkSize(0.0), visualSelectionLength(0),
    selNumChunks(), selPlayMode(),
    isLatchedBtn("LATCH"), isReversedBtn("NORM"), times2("x2"), div2("/2"),
    speedLockBtn("speed lock", DrawableButton::ImageRaw), isSpeedLocked(false),
    lockImg(), unlockImg(),
    stripVolumeSldr("strip volume"),
    selectionPointToChange(0),
    mouseDownMods(),
    mlrVSTEditor(owner), dataStrip(dataStripLink),
    menuLF()
{
    // load binary data for lock icon
    lockImg.setImage(ImageCache::getFromMemory(BinaryData::locked_png, BinaryData::locked_pngSize));
    unlockImg.setImage(ImageCache::getFromMemory(BinaryData::unlocked_png, BinaryData::unlocked_pngSize));

    // does the heavy UI positioning   
    buildUI();
    buildNumBlocksList(8);

    
    dataStrip->addChangeListener(this);

    // listen for user input
    selPlayMode.addListener(this);
    playbackSpeedSldr.addListener(this);
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
        // so just stop the sample first instead!
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
            mlrVSTEditor->switchChannels(i, sampleStripID);
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
        mlrVSTEditor->modPlaySpeed(2.0, sampleStripID);
    }
    else if (btn == &div2)
    {
        mlrVSTEditor->modPlaySpeed(0.5, sampleStripID);
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
    else if(sldr == &playbackSpeedSldr)
    {
        double newPlaySpeed = playbackSpeedSldr.getValue();
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

    playbackSpeedSldr.setColour(Slider::thumbColourId, backgroundColour);
    playbackSpeedSldr.setColour(Slider::backgroundColourId, backgroundColour.darker());

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
        Colour chanColour = mlrVSTEditor->getChannelColour(chan);
        channelButtonArray.getLast()->setBackgroundColours(chanColour, chanColour);
    }

    int previousChannel = *static_cast<const int*>(dataStrip->getSampleStripParam(SampleStrip::pCurrentChannel));
    if (previousChannel >= numChannels){
        DBG("Current channel outside range: resetting to channel 0.");
        updateChannelColours(0);
        mlrVSTEditor->switchChannels(0, sampleStripID);
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

    addAndMakeVisible(&playSpeedLbl);
    playSpeedLbl.setBounds(newXposition, 0, 64, controlbarSize);
    playSpeedLbl.setColour(Label::backgroundColourId, Colours::black);
    playSpeedLbl.setColour(Label::textColourId, Colours::white);
    playSpeedLbl.setFont(fontSize);

    newXposition += 64;

    addAndMakeVisible(&playbackSpeedSldr);
    playbackSpeedSldr.setSliderStyle(Slider::LinearBar);
    playbackSpeedSldr.setColour(Slider::textBoxTextColourId, Colours::white);
    playbackSpeedSldr.setBounds(newXposition, 0, 80, controlbarSize);
    playbackSpeedSldr.setRange(0.0, 4.0, 0.001);
    playbackSpeedSldr.setTextBoxIsEditable(true);
    playbackSpeedSldr.setLookAndFeel(&menuLF);
    

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

    if (e.mods == ModifierKeys::rightButtonModifier && e.y > controlbarSize)
    {

        int currentSamplePoolSize = mlrVSTEditor->getSamplePoolSize(mlrVSTAudioProcessor::pSamplePool);
        int currentResamplePoolSize = mlrVSTEditor->getSamplePoolSize(mlrVSTAudioProcessor::pResamplePool);
        int currentRecordPoolSize = mlrVSTEditor->getSamplePoolSize(mlrVSTAudioProcessor::pRecordPool);

        // TODO: can this be cached and only repopulated when the sample pool changes?
        // TODO: middle click to delete sample under cursor in menu?
        PopupMenu p = PopupMenu();
        PopupMenu resamplePoolMenu = PopupMenu();
        PopupMenu recordPoolMenu = PopupMenu();

        int menuCounter = 1;        // tracks how many items in the menu
        p.addItem(menuCounter, "None");
        

        for (int m = 0; m < currentResamplePoolSize; ++m)
        {
            ++menuCounter;
            resamplePoolMenu.addItem(menuCounter, "resample " + String(m));
        }
        
        for (int m = 0; m < currentRecordPoolSize; ++m)
        {
            ++menuCounter;
            recordPoolMenu.addItem(menuCounter, "record " + String(m));
        }

        
        p.addSubMenu("resample", resamplePoolMenu);
        p.addSubMenu("record", recordPoolMenu);


        // for each sample, add it to the list
        for (int i = 0; i < currentSamplePoolSize; ++i)
        {
            // +1 because 0 is result for no item clicked
            // +1 because "none" is also an option
            String iFileName = mlrVSTEditor->getSampleName(i, mlrVSTAudioProcessor::pSamplePool);

            ++menuCounter;
            p.addItem(menuCounter, iFileName);
        }

        // show the menu and store choice
        int fileChoice = p.showMenu(PopupMenu::Options().withTargetComponent(&chanLbl));


        // If "none is selected"
        if (fileChoice == 1)
        {
            dataStrip->stopSamplePlaying();

            const AudioSample *newSample = 0;
            dataStrip->setSampleStripParam(SampleStrip::pAudioSample, newSample);

        }
        // If a menu option has been chosen that is a file
        else if (fileChoice != 0)
        {
            // -1 as everyitem is offset by 1
            // -1 to correct for "none" option
            fileChoice -= 2;

            // something is selected from the resample menu
            if (fileChoice < currentResamplePoolSize && fileChoice >= 0)
            {
                DBG("resample option: "<< fileChoice);
                selectNewSample(fileChoice, mlrVSTAudioProcessor::pResamplePool);
                return;
            }
            else fileChoice -= currentResamplePoolSize;

            // something is selected from the record menu
            if (fileChoice < currentRecordPoolSize && fileChoice >= 0)
            {
                DBG("record option: "<< fileChoice);
                selectNewSample(fileChoice, mlrVSTAudioProcessor::pRecordPool);
                return;
            }
            else fileChoice -= currentRecordPoolSize;

            // an audio sample has been selected
            if(fileChoice < currentSamplePoolSize && fileChoice >= 0)
            {
                DBG("sample option: " << fileChoice);
                selectNewSample(fileChoice, mlrVSTAudioProcessor::pSamplePool);
            }
            
        }


    }

    // middle click-drag to move the selection
    else if (e.mods == ModifierKeys::middleButtonModifier)
    {
        selectionStartBeforeDrag = visualSelectionStart;
    }

    // ctrl-left and ctrl-shift-left move one of the end points
    else if ((e.mods == (ModifierKeys::ctrlModifier + 
                         ModifierKeys::leftButtonModifier)) ||
             (e.mods == (ModifierKeys::ctrlModifier + 
                         ModifierKeys::leftButtonModifier + 
                         ModifierKeys::shiftModifier)))
    {
        if ( abs(e.x - visualSelectionStart) > abs(e.x - visualSelectionEnd) )
            selectionPointToChange = &visualSelectionEnd;
        else
            selectionPointToChange = &visualSelectionStart;
        dragStart = e.x;

        DBG("storing start");
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
        float start = 0.0f, end = 1.0f;
        dataStrip->setSampleStripParam(SampleStrip::pFractionalStart, &start);
        dataStrip->setSampleStripParam(SampleStrip::pFractionalEnd, &end);

        mlrVSTEditor->calcInitialPlaySpeed(sampleStripID);

        // repaint when we next get a timed update
        stripChanged = true;
    }
}

void SampleStripControl::mouseUp(const MouseEvent &e)
{
    if (e.mods == ModifierKeys::leftButtonModifier)
    {
        //// try to send the new selection to the SampleStrips
        //mlrVSTAudioProcessorEditor *pluginUI = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);
        //// let the sample strip know about the change
        //if (pluginUI != 0)
        //{
        //    float fractionalStart = visualSelectionStart / (float) componentWidth;
        //    float fractionalEnd = visualSelectionEnd / (float) componentWidth;
        //    pluginUI->setSampleStripParameter(SampleStrip::pFractionalStart, &fractionalStart, sampleStripID);
        //    pluginUI->setSampleStripParameter(SampleStrip::pFractionalEnd, &fractionalEnd, sampleStripID);
        //}

        //// redraw to reflect new selection
        //repaint();
    }
}

void SampleStripControl::mouseDrag(const MouseEvent &e)
{
    if (mouseDownMods == ModifierKeys::leftButtonModifier)
    {

        int mouseX = e.x;

        // Make sure we don't select outside the waveform
        if (mouseX > componentWidth) mouseX = componentWidth;
        if (mouseX < 0) mouseX = 0;

        visualSelectionStart = e.getMouseDownX();
        visualSelectionEnd = mouseX;

    }
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
    else if (mouseDownMods == (ModifierKeys::ctrlModifier + ModifierKeys::leftButtonModifier))
    {
        // Don't select outside the component!
        int newValue = e.x;
        if (newValue < 0) newValue = 0;
        else if (newValue > componentWidth) newValue = componentWidth;

        *selectionPointToChange = newValue;
    }

    ///* SNAPPING CODE, not yet working */
    //else if ((e.mods == (ModifierKeys::ctrlModifier + 
    //                     ModifierKeys::leftButtonModifier +
    //                     ModifierKeys::shiftModifier)))
    //{
    //    // TODO: have snapping interval as an option
    //    float eighth = componentWidth / 8.0;
    //    float snapPos = fmod(e.x, eighth);
    //    int newSeg = (int) (e.x / eighth);

    //    if (snapPos > eighth / 2.0)
    //    {
    //        newSeg += 1;
    //    }
    //    *selectionPointToChange = (int)(newSeg * eighth);
    //}
    //DBG(visualSelectionStart);
    //DBG(visualSelectionEnd);


    // Swap if inverse selection
    if (visualSelectionEnd < visualSelectionStart)
    {
        int temp = visualSelectionStart;
        visualSelectionStart = visualSelectionEnd;
        visualSelectionEnd = temp;
    }

    visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
    visualChunkSize = (visualSelectionLength / (float) numChunks);

    // try to send the new selection to the SampleStrips
    float fractionalStart = visualSelectionStart / (float) componentWidth;
    float fractionalEnd = visualSelectionEnd / (float) componentWidth;
    dataStrip->setSampleStripParam(SampleStrip::pFractionalStart, &fractionalStart);
    dataStrip->setSampleStripParam(SampleStrip::pFractionalEnd, &fractionalEnd);

    if (mouseDownMods != ModifierKeys::middleButtonModifier)
        mlrVSTEditor->updatePlaySpeedForNewSelection(sampleStripID);

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
        g.setColour(Colours::white);
        g.drawFittedText("Playback at " + String(playbackPercentage), 0,
                         controlbarSize, componentWidth,
                         componentHeight - controlbarSize, Justification::centred, 2);
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
        fileIndex = mlrVSTEditor->loadSampleFromFile(currentSampleFile);
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
    const AudioSample * newSample = mlrVSTEditor->getAudioSample(fileChoice, poolID);
    dataStrip->setSampleStripParam(SampleStrip::pAudioSample, newSample);

    // select whole sample by default
    visualSelectionStart = 0;
    visualSelectionEnd = componentWidth;
    visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
    visualChunkSize = (visualSelectionLength / (float) numChunks);

    // update the selection
    float start = 0.0f, end = 1.0f;
    dataStrip->setSampleStripParam(SampleStrip::pFractionalStart, &start);
    dataStrip->setSampleStripParam(SampleStrip::pFractionalEnd, &end);

    // and try to find the best playback speed (i.e. closest to 1).
    mlrVSTEditor->calcInitialPlaySpeed(sampleStripID);

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


    case SampleStrip::pFractionalStart :
        {
            float fractionalStart = *static_cast<const float*>(newValue);
            visualSelectionStart = (int)(fractionalStart * componentWidth);
            visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
            visualChunkSize = (visualSelectionLength / (float) numChunks);
            break;
        }

    case SampleStrip::pFractionalEnd :
        {
            float fractionalEnd = *static_cast<const float*>(newValue);
            visualSelectionEnd = (int)(fractionalEnd * componentWidth);
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
            playbackSpeedSldr.setValue(newPlaySpeed, false);
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
                    playbackSpeedSldr.setEnabled(false);
                }
                else
                {
                    speedLockBtn.setImages(&unlockImg);
                    playbackSpeedSldr.setEnabled(true);
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
                    playbackSpeedSldr.setValue(1.0, true);
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
