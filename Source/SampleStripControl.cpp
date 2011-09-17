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

SampleStripControl::SampleStripControl(const int &id,
                                       const int &width,
                                       const int &height,
                                       const int &newNumChannels,
                                       mlrVSTAudioProcessorEditor * const owner) :
    componentHeight(height), componentWidth(width), sampleStripID(id),
    waveformPaintBounds(0, 15, componentWidth, componentHeight - 15),
    backgroundColour(Colours::black),
    thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache),
    thumbnailLength(0.0),
    trackNumberLbl("track number", String(sampleStripID)),
    filenameLbl("filename", "No File"),
    numChannels(newNumChannels),
    channelButtonArray(),
    isReversed(false),
    numChunks(8), currentSample(0),
    visualSelectionStart(0), visualSelectionEnd(componentWidth), visualChunkSize(0.0), visualSelectionLength(0),
    selNumChunks(), selPlayMode(),
    isLatchedBtn("latch"), isReversedBtn("reversed"),
    stripVolumeSldr("strip volume"),
    selectionPointToChange(0),
    mouseDownMods(),
    mlrVSTEditor(owner)
{

    addAndMakeVisible(&filenameLbl);
    filenameLbl.setColour(Label::backgroundColourId, Colours::white);
    filenameLbl.setColour(Label::textColourId, Colours::black);
    filenameLbl.setJustificationType(Justification::right);

    filenameLbl.setBounds(0, 0, componentWidth, 15);
    filenameLbl.setFont(10.0f);

    addAndMakeVisible(&trackNumberLbl);
    trackNumberLbl.setBounds(0, 0, 15, 15);
    trackNumberLbl.setColour(Label::backgroundColourId, Colours::black);
    trackNumberLbl.setColour(Label::textColourId, Colours::white);
    trackNumberLbl.setFont(10.0f);


    // this will eventually be loaded from a setting
    buildNumBlocksList(8);
    addAndMakeVisible(&selNumChunks);
    selNumChunks.addListener(this);
    selNumChunks.setBounds(200, 0, 30, 15);

    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);

    addAndMakeVisible(&selPlayMode);
    selPlayMode.addListener(this);
    selPlayMode.addItem("loop", SampleStrip::LOOP);
    selPlayMode.addItem("loop chunk", SampleStrip::LOOP_CHUNK);
    selPlayMode.addItem("play to end", SampleStrip::PLAY_TO_END);
    selPlayMode.setBounds(250, 0, 50, 15);

    addAndMakeVisible(&isLatchedBtn);
    isLatchedBtn.setBounds(350, 0, 50, 15);
    isLatchedBtn.addListener(this);

    addAndMakeVisible(&isReversedBtn);
    isReversedBtn.setBounds(450, 0, 70, 15);
    isReversedBtn.addListener(this);

    addAndMakeVisible(&stripVolumeSldr);
    stripVolumeSldr.setSliderStyle(Slider::LinearBar);
    stripVolumeSldr.setColour(Slider::textBoxTextColourId, Colours::white);
    stripVolumeSldr.setBounds(500, 0, 50, 15);
    stripVolumeSldr.setRange(0.0, 2.0, 0.01);
    stripVolumeSldr.addListener(this);
}

SampleStripControl::~SampleStripControl()
{
    thumbnail.removeChangeListener(this);
}

void SampleStripControl::buildNumBlocksList(const int &newMaxNumBlocks)
{
    selNumChunks.clear();
    for (int i = 1; i <= newMaxNumBlocks; ++i)
        selNumChunks.addItem(String(i), i);

    // select the max number of blocks (comboBoxChanged will 
    // be informed).
    // selNumChunks.setSelectedId(newMaxNumBlocks);
}

void SampleStripControl::changeListenerCallback(ChangeBroadcaster*)
{
    // this method is called by the thumbnail when it has changed, so we should repaint it..
    repaint();
}

void SampleStripControl::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{

    if (comboBoxThatHasChanged == &selNumChunks)
    {
        numChunks = selNumChunks.getSelectedId();
        visualChunkSize = (visualSelectionLength / (float) numChunks);
        mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamNumChunks,
                                             &numChunks,
                                             sampleStripID);
        repaint();
    }
    else if (comboBoxThatHasChanged == &selPlayMode)
    {
        int newPlayMode = selPlayMode.getSelectedId();
        mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamPlayMode,
                                             &newPlayMode, sampleStripID);
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
            setChannel(i);
        }
    }

    if (btn == &isLatchedBtn)
    {
        isLatched = !isLatched;
        mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamIsLatched, &isLatched, sampleStripID);
    }
    else if (btn == &isReversedBtn)
    {
        isReversed = !isReversed;
        mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamIsReversed, &isReversed, sampleStripID);
        String newButtonText = (isReversed) ? "reversed" : "normal";
        isReversedBtn.setButtonText(newButtonText);
    }
}

void SampleStripControl::sliderValueChanged(Slider *sldr)
{
    if (sldr == &stripVolumeSldr)
    {
        float newStripVol = (float)(stripVolumeSldr.getValue());
        mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamStripVolume, &newStripVol, sampleStripID);
    }
}

void SampleStripControl::setChannel(const int &newChannel)
{
    // ...so we can let the processor know
    mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamCurrentChannel, &newChannel, sampleStripID);

    backgroundColour = channelButtonArray[newChannel]->getBackgroundColour();
    stripVolumeSldr.setColour(Slider::thumbColourId, backgroundColour);
    stripVolumeSldr.setColour(Slider::backgroundColourId, backgroundColour.darker());
    repaint();
}

// This is particuarly usful if the number of channels changes
void SampleStripControl::buildChannelButtonList(const int &newNumChannels)
{
    numChannels = newNumChannels;
    // clear existing buttons
    channelButtonArray.clear();

    for(int chan = 0; chan < numChannels; ++chan)
    {
        channelButtonArray.add(new DrawableButton("button" + String(chan), DrawableButton::ImageRaw));
        addAndMakeVisible(channelButtonArray.getLast());
        channelButtonArray.getLast()->setBounds(15 + chan * 15, 0, 15, 15);
        channelButtonArray.getLast()->addListener(this);
        Colour chanColour = mlrVSTEditor->getChannelColour(chan);
        channelButtonArray.getLast()->setBackgroundColours(chanColour, chanColour);
    }


    int previousChannel = *static_cast<const int*>(mlrVSTEditor->getSampleStripParameter(SampleStrip::ParamCurrentChannel, sampleStripID));
    if (previousChannel >= numChannels){
        DBG("Current channel outside range: resetting to channel 0.");
        setChannel(0);
    }
}


void SampleStripControl::setZoomFactor(double /*amount*/)
{
    //if (thumbnail.getTotalLength() > 0)
    //{
    //    double timeDisplayed = jmax (0.001, (thumbnail.getTotalLength() - startTime) * (1.0 - jlimit (0.0, 1.0, amount)));
    //    endTime = startTime + timeDisplayed;
    //    repaint();
    //}
}

void SampleStripControl::mouseWheelMove(const MouseEvent&, float /*wheelIncrementX*/, float /*wheelIncrementY*/)
{
    //if (thumbnail.getTotalLength() > 0)
    //{
    //    double newStart = startTime + (wheelIncrementX + wheelIncrementY) * (endTime - startTime) / 10.0;
    //    newStart = jlimit (0.0, thumbnail.getTotalLength() - (endTime - startTime), newStart);
    //    endTime = newStart + (endTime - startTime);
    //    startTime = newStart;
    //    repaint();
    //}
}

void SampleStripControl::mouseDown(const MouseEvent &e)
{
    // Save modifers of the initial press
    mouseDownMods = e.mods;

    if (e.mods == ModifierKeys::rightButtonModifier && e.y > 15)
    {

        int currentSamplePoolSize = mlrVSTEditor->getSamplePoolSize();

        // only show the menu if we have samples in the pool
        if (currentSamplePoolSize != 0)
        {
            // TODO: can this be cached and only repopulated when the sample pool changes?
            // TODO: middle click to delete sample under cursor in menu?

            PopupMenu p = PopupMenu();

            p.addItem(1, "None");
            // for each sample, add it to the list
            for (int i = 0; i < currentSamplePoolSize; ++i)
            {
                // +1 because 0 is result for no item clicked
                // +1 because "none" is also an option
                String iFileName = mlrVSTEditor->getSampleName(i);
                p.addItem(i + 2, iFileName);
            }

            // show the menu and store choice
            int fileChoice = p.showMenu(PopupMenu::Options().withTargetComponent(&filenameLbl));

            // If "none is selected"
            if (fileChoice == 1)
            {
                const AudioSample *newSample = 0;
                mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamAudioSample, newSample, sampleStripID);

                bool isPlaying = false;
                mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamIsPlaying, &isPlaying, sampleStripID);
            }
            // If a menu option has been chosen that is a file
            else if (fileChoice != 0)
            {
                // -1 to correct for +1 in for loop above
                // -1 to correct for "none" option
                fileChoice -= 2;

                // and let the audio processor update the sample strip
                mlrVSTEditor->updateSampleStripSample(fileChoice, sampleStripID);

                // select whole sample by default
                visualSelectionStart = 0;
                visualSelectionEnd = componentWidth;
                visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
                visualChunkSize = (visualSelectionLength / (float) numChunks);

                // update the selection
                float start = 0.0f, end = 1.0f;
                mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamFractionalStart, &start, sampleStripID);
                mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamFractionalEnd, &end, sampleStripID);

                repaint();
            }
        }

    }
    else if (e.mods == ModifierKeys::middleButtonModifier)
    {
        selectionStartBeforeDrag = visualSelectionStart;
    }
    else if (e.mods == (ModifierKeys::ctrlModifier + ModifierKeys::leftButtonModifier))
    {
        if ( abs(e.x - visualSelectionStart) > abs(e.x - visualSelectionEnd) )
            selectionPointToChange = &visualSelectionEnd;
        else
            selectionPointToChange = &visualSelectionStart;
        dragStart = e.x;
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
        //    pluginUI->setSampleStripParameter(SampleStrip::ParamFractionalStart, &fractionalStart, sampleStripID);
        //    pluginUI->setSampleStripParameter(SampleStrip::ParamFractionalEnd, &fractionalEnd, sampleStripID);
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
        //int mouseStartX = e.getMouseDownX();

        // Make sure we don't select outside the waveform
        if (mouseX > componentWidth) mouseX = componentWidth;
        if (mouseX < 0) mouseX = 0;

        visualSelectionStart = e.getMouseDownX();
        visualSelectionEnd = mouseX;
        //if (mouseX > mouseStartX)
        //{
        //    visualSelectionStart = mouseStartX;
        //    visualSelectionEnd = mouseX;
        //}
        //else
        //{
        //    visualSelectionStart = mouseX;
        //    visualSelectionEnd = mouseStartX;
        //}


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

        // TODO: This will crash if control is pressing during drag
        *selectionPointToChange = newValue;
    }

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
    mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamFractionalStart, &fractionalStart, sampleStripID);
    mlrVSTEditor->setSampleStripParameter(SampleStrip::ParamFractionalEnd, &fractionalEnd, sampleStripID);

    // redraw to reflect new selection
    repaint();
}

void SampleStripControl::paint(Graphics& g)
{
    // Start with the background colour
    g.fillAll(backgroundColour);

    if (isPlaying)
    {
        g.setColour(Colours::black.withAlpha(0.15f));
        int visualPlaybackPoint = (int)(playbackPercentage * visualSelectionLength);
        g.fillRect(visualSelectionStart, 15, visualPlaybackPoint, componentHeight - 15);

        g.setColour(Colours::white);
        g.drawFittedText("Playback at " + String(playbackPercentage), 0, 15, componentWidth, componentHeight - 15, Justification::centred, 2);
    }

    // Draw the current sample waveform in white
    g.setColour(Colours::white);

    if(currentSample && thumbnailLength > 0)
    {
        thumbnail.drawChannels(g, waveformPaintBounds, 0, thumbnailLength, 1.0f);
    }
    else
    {
        g.setFont(12.0f);
        g.drawFittedText("(No audio file selected)", 0, 15, componentWidth, componentHeight - 15, Justification::centred, 2);
    }



    /* Finally we grey out the parts of the sample which aren't in
       the selection and paint stripes to indicate what button 
       will do what.
    */
    g.setColour(Colours::black.withAlpha(0.6f));
    g.fillRect(0, 15, visualSelectionStart, componentHeight - 15); 
    g.fillRect(visualSelectionEnd, 15, componentWidth - visualSelectionEnd, componentHeight - 15); 

    /* Add alternate strips to make it clear which
    button plays which part of the sample. */
    g.setColour(Colours::black.withAlpha(0.1f));
    for(int i = 1; i < numChunks; i+=2)
    {
        g.fillRect((float) (visualSelectionStart + i * visualChunkSize),
            15.0f, visualChunkSize, componentHeight - 15.0f);
    }

}


void SampleStripControl::filesDropped(const StringArray& files, int /*x*/, int /*y*/)
{
    // try to add each of the loaded files to the sample pool
    for (int i = 0; i < files.size(); ++i)
    {
        File currentSampleFile(files[i]);
        DBG("Dragged file: " << files[i]);

        mlrVSTEditor->loadSampleFromFile(currentSampleFile);

        // try to load the sample
        //if (pluginUI->loadSampleFromFile(currentSampleFile))
        //{
            // MAJORTODO: this fails when exising files loaded
            //updateThumbnail(currentSampleFile);
            //int samplePoolID = pluginUI->getSamplePoolSize();
            //pluginUI->updateSampleStripSample(samplePoolID - 1, sampleStripID);
        //}
    }

    // set latest sample in the pool as the current sample
    // DESIGN: is this correct behaviour?
    //if(pluginUI->getLatestSample() != 0) setAudioSample(pluginUI->getLatestSample());
}


void SampleStripControl::recallParam(const int &paramID, const void *newValue, const bool &doRepaint)
{
    switch (paramID)
    {
    case SampleStrip::ParamCurrentChannel :
        {
            int newCurrentChannel = *static_cast<const int*>(newValue);
            setChannel(newCurrentChannel);
            break;
        }

    case SampleStrip::ParamNumChunks :
        {
            const int newNumChunks = *static_cast<const int*>(newValue);
            selNumChunks.setSelectedId(newNumChunks);
            visualChunkSize = (float)(visualSelectionLength / (float) newNumChunks);
            break;
        }

    case SampleStrip::ParamPlayMode :
        {
            int newPlayMode = *static_cast<const int*>(newValue);
            selPlayMode.setSelectedId(newPlayMode);
            break;
        }

    case SampleStrip::ParamIsLatched :
        {
            isLatched = *static_cast<const bool*>(newValue);
            isLatchedBtn.setToggleState(isLatched, false);
            break;
        }

    case SampleStrip::ParamIsReversed :
        {
            isReversed = *static_cast<const bool*>(newValue);
            isReversedBtn.setToggleState(isReversed, false);
            break;
        }

    case SampleStrip::ParamIsPlaying :
        isPlaying = *static_cast<const bool*>(newValue);
        break;

    case SampleStrip::ParamPlaybackPercentage :
        playbackPercentage = *static_cast<const float*>(newValue);
        break;


    case SampleStrip::ParamFractionalStart :
        {
            float fractionalStart = *static_cast<const float*>(newValue);
            visualSelectionStart = (int)(fractionalStart * componentWidth);
            visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
            visualChunkSize = (visualSelectionLength / (float) numChunks);
            break;
        }

    case SampleStrip::ParamFractionalEnd :
        {
            float fractionalEnd = *static_cast<const float*>(newValue);
            visualSelectionEnd = (int)(fractionalEnd * componentWidth);
            visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
            visualChunkSize = (visualSelectionLength / (float) numChunks);
            break;
        }

    case SampleStrip::ParamStripVolume :
        {
            float newStripVolume = *static_cast<const float*>(newValue);
            stripVolumeSldr.setValue(newStripVolume);
            break;
        }

    case SampleStrip::ParamAudioSample :
        {
            const AudioSample *newSample = static_cast<const AudioSample*>(newValue);

            // Only update if we have to as reloading thumbnail is expensive
            if (newSample != currentSample )
            {
                currentSample = newSample;
                // If the new sample exists
                if (newSample)
                {
                    thumbnail.setSource(new FileInputSource(currentSample->getSampleFile()));
                    thumbnailLength = thumbnail.getTotalLength();
                    filenameLbl.setText(currentSample->getSampleName(), false);
                }
                else filenameLbl.setText("No file", false);
            }
            
            break;
        }

    default :
        jassert(false);
        DBG("Param not found");
    }

    if(doRepaint) repaint();
}
