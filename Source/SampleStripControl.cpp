/*
==============================================================================

SampleStripControl.cpp
Created: 6 Sep 2011 12:38:13pm
Author:  Hemmer

Custom component to display a waveform (corresponding to an mlr row)

==============================================================================
*/


#include "PluginEditor.h"

SampleStripControl::SampleStripControl(const int &id,
                                       const int &width,
                                       const int &height,
                                       const int &newNumChannels) :
    componentHeight(height), componentWidth(width),
    waveformPaintBounds(0, 15, componentWidth, componentHeight - 15),
    thumbnailCache(5),
    thumbnail(512, formatManager, thumbnailCache),
    backgroundColour(Colours::black),
    trackNumberLbl("track number", String(sampleStripID)),
    filenameLbl("filename", "No File"),
    sampleStripID(id),
    numChannels(newNumChannels),
    channelButtonArray(),
    isReversed(false),
    numChunks(8),
    thumbnailLength(0.0),
    visualSelectionStart(0), visualSelectionEnd(0), visualChunkSize(0.0), visualSelectionLength(0),
    selNumChunks(), selPlayMode()
{

    addAndMakeVisible(&filenameLbl);
    filenameLbl.setColour(Label::backgroundColourId, Colours::white);
    filenameLbl.setColour(Label::textColourId, Colours::black);
    filenameLbl.setJustificationType(Justification::right);

    filenameLbl.setBounds(0, 0, componentWidth, 15);
    filenameLbl.setFont(10.0f);

    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);

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



    addAndMakeVisible(&selPlayMode);
    selPlayMode.addListener(this);
    selPlayMode.addItem("loop", SampleStrip::LOOP);
    selPlayMode.addItem("loop while held", SampleStrip::LOOP_WHILE_HELD);
    selPlayMode.addItem("loop chunk", SampleStrip::LOOP_CHUNK);
    selPlayMode.addItem("play to end", SampleStrip::PLAY_TO_END);
    selPlayMode.setBounds(250, 0, 50, 15);
    // set LOOP as default mode and send a change message
    // selPlayMode.setSelectedId(SampleStrip::LOOP, false);
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

void SampleStripControl::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    // get pointer to parent class
    mlrVSTAudioProcessorEditor* pluginUI = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);


    if (comboBoxThatHasChanged == &selNumChunks)
    {
        numChunks = selNumChunks.getSelectedId();
        visualChunkSize = (visualSelectionLength / (float) numChunks);
        pluginUI->setSampleStripParameter(SampleStrip::ParamNumChunks,
                                             &numChunks,
                                             sampleStripID);
        repaint();
    }
    else if (comboBoxThatHasChanged == &selPlayMode)
    {
        int newPlayMode = selPlayMode.getSelectedId();
        pluginUI->setSampleStripParameter(SampleStrip::ParamPlayMode,
                                             &newPlayMode, sampleStripID);
    }

}

SampleStripControl::~SampleStripControl()
{
    thumbnail.removeChangeListener(this);
}

void SampleStripControl::buttonClicked(Button *btn)
{
    
    // see if any of the channel selection buttons were chosen
    for (int i = 0; i < channelButtonArray.size(); ++i)
    {
        if (channelButtonArray[i] == btn)
        {
            
            // update the GUI
            setChannel(i);
        }
    }
}

void SampleStripControl::setChannel(const int &newChannel)
{
    // Get pointer to parent class...
    mlrVSTAudioProcessorEditor* pluginUI = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);
    // ...so we can let the processor know
    pluginUI->setSampleStripParameter(SampleStrip::ParamCurrentChannel, &newChannel, sampleStripID);

    DBG("#1 channel is now " << newChannel << " " << sampleStripID);
    backgroundColour = channelButtonArray[newChannel]->getBackgroundColour();
    repaint();
}

// This is particuarly usful if the number of channels changes
void SampleStripControl::buildChannelButtonList(const int &newNumChannels)
{
    numChannels = newNumChannels;
    // clear existing buttons
    channelButtonArray.clear();

    mlrVSTAudioProcessorEditor *pluginUI = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);

    for(int chan = 0; chan < numChannels; ++chan)
    {
        channelButtonArray.add(new DrawableButton("button" + String(chan), DrawableButton::ImageRaw));
        addAndMakeVisible(channelButtonArray.getLast());
        channelButtonArray.getLast()->setBounds(15 + chan * 15, 0, 15, 15);
        channelButtonArray.getLast()->addListener(this);
        Colour chanColour = pluginUI->getChannelColour(chan);
        channelButtonArray.getLast()->setBackgroundColours(chanColour, chanColour);
    }

    DBG("resetting to chan 0");
    int previousChannel = *static_cast<const int*>(pluginUI->getSampleStripParameter(SampleStrip::ParamCurrentChannel, sampleStripID));
    if (previousChannel >= numChannels){
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

    if (e.mods == ModifierKeys::rightButtonModifier && e.y > 15)
    {
        mlrVSTAudioProcessorEditor *demoPage = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);

        if (demoPage != 0)
        {
            int currentSamplePoolSize = demoPage->getSamplePoolSize();

            // only show the menu if we have samples in the pool
            if (currentSamplePoolSize != 0)
            {
                // TODO: can this be cached and only repopulated when the sample pool changes?
                PopupMenu p = PopupMenu();
                
                // TODO: add option to select "no file?
                // TODO: middle click to delete sample under cursor in menu?

                // for each sample, add it to the list
                for (int i = 0; i < currentSamplePoolSize; ++i)
                {
                    // +1 is because 0 is result for no item clicked
                    String iFileName = demoPage->getSampleName(i);
                    p.addItem(i + 1, iFileName);
                }

                // show the menu and store choice
                int fileChoice = p.showMenu(PopupMenu::Options().withTargetComponent(&filenameLbl));

                // if a menu option has been chosen
                if (fileChoice != 0)
                {
                    // -1 is to correct for +1 in for loop above
                    --fileChoice;



                    // update thumbnails
                    updateThumbnail(demoPage->getSampleSourceFile(fileChoice));
                    // and let the audio processor update the sample strip
                    demoPage->updateSampleStripSample(fileChoice, sampleStripID);

                    // select whole sample by default
                    visualSelectionStart = 0;
                    visualSelectionEnd = componentWidth;
                    visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
                    visualChunkSize = (visualSelectionLength / (float) numChunks);
                    
                    // update the selection
                    float start = 0.0f, end = 1.0f;
                    demoPage->setSampleStripParameter(SampleStrip::ParamFractionalStart, &start, sampleStripID);
                    demoPage->setSampleStripParameter(SampleStrip::ParamFractionalEnd, &end, sampleStripID);
                        

                    repaint();
                }
            }
        }
    }
    //else if(e.mods = ModifierKeys::leftButtonModifier)
    //{
    //    visualSelectionStart = e.getMouseDownX();
    //    visualSelectionEnd = visualSelectionStart;
    //}
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
    if (e.mods == ModifierKeys::leftButtonModifier)
    {

        int mouseX = e.x;
        int mouseStartX = e.getMouseDownX();

        // Make sure we don't select outside the waveform
        if (mouseX > componentWidth) mouseX = componentWidth;
        if (mouseX < 0) mouseX = 0;

        if (mouseX > mouseStartX)
        {
            visualSelectionStart = mouseStartX;
            visualSelectionEnd = mouseX;
        }
        else
        {
            visualSelectionStart = mouseX;
            visualSelectionEnd = mouseStartX;
        }

        visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
        visualChunkSize = (visualSelectionLength / (float) numChunks);
    }

    // try to send the new selection to the SampleStrips
    mlrVSTAudioProcessorEditor *pluginUI = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);
    if (pluginUI != 0)
    {
        float fractionalStart = visualSelectionStart / (float) componentWidth;
        float fractionalEnd = visualSelectionEnd / (float) componentWidth;
        pluginUI->setSampleStripParameter(SampleStrip::ParamFractionalStart, &fractionalStart, sampleStripID);
        pluginUI->setSampleStripParameter(SampleStrip::ParamFractionalEnd, &fractionalEnd, sampleStripID);
    }

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
    if (thumbnail.getTotalLength() > 0)
    {
        thumbnail.drawChannels(g, waveformPaintBounds, 0, thumbnailLength, 1.0f);
    }
    else
    {
        g.setFont(12.0f);
        g.drawFittedText("(No audio file selected)", 0, 15, componentWidth, componentHeight - 15, Justification::centred, 2);
    }



    /* Finally we grey out the parts of the sample which aren't in
    the selection. The if statements are because we need to be
    able to paint the strip even if the selection is made backwards!
    */
    g.setColour(Colours::black.withAlpha(0.6f));
    if (visualSelectionEnd > visualSelectionStart)
    {
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
    else
    {
        g.fillRect(0, 15, visualSelectionEnd, componentHeight - 15); 
        g.fillRect(visualSelectionStart, 15, componentWidth - visualSelectionStart, componentHeight - 15); 
        /* Add alternate strips to make it clear which
        button plays which part of the sample. */
        g.setColour(Colours::black.withAlpha(0.1f));
        for(int i = 1; i < numChunks; i+=2)
        {
            g.fillRect((float) (visualSelectionEnd + i * visualChunkSize),
                15.0f, visualChunkSize, componentHeight - 15.0f);
        }
    }



}

void SampleStripControl::changeListenerCallback(ChangeBroadcaster*)
{
    // this method is called by the thumbnail when it has changed, so we should repaint it..
    repaint();
}

bool SampleStripControl::isInterestedInFileDrag(const StringArray& /*files*/)
{
    return true;
}

void SampleStripControl::filesDropped(const StringArray& files, int /*x*/, int /*y*/)
{
    // get pointer to parent class
    mlrVSTAudioProcessorEditor* pluginUI = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);

    // try to add each of the loaded files to the sample pool
    for (int i = 0; i < files.size(); ++i)
    {
        File currentSampleFile(files[i]);
        DBG("Dragged file: " << files[i]);

        pluginUI->loadSampleFromFile(currentSampleFile);

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

void SampleStripControl::updateThumbnail(const File &newFile)
{
    thumbnail.setSource(new FileInputSource(newFile));
    thumbnailLength = thumbnail.getTotalLength();

    // update filename label
    filenameLbl.setText(newFile.getFullPathName(), false);

    DBG("Waveform strip " + String(sampleStripID) + ": file \"" + newFile.getFileName() + "\" selected.");
}


void SampleStripControl::recallParam(const int &paramID, const void *newValue, const bool &doRepaint)
{
    switch (paramID)
    {
    case SampleStrip::ParamCurrentChannel :
        {
            int newCurrentChannel = *static_cast<const int*>(newValue);
            setChannel(newCurrentChannel);
            DBG("From SampleStrip: channel is now " + String(newCurrentChannel));
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
            selPlayMode.setSelectedId(newPlayMode); break;
        }

    case SampleStrip::ParamIsReversed :
        {
            bool newIsReversed = *static_cast<const bool*>(newValue);
            isReversed = newIsReversed; break;
        }

    case SampleStrip::ParamIsPlaying :
        isPlaying = *static_cast<const bool*>(newValue); break;

    case SampleStrip::ParamPlaybackPercentage :
        playbackPercentage = *static_cast<const float*>(newValue); break;


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

    default :
        jassert(false);
        DBG("Param not found");
    }

    if(doRepaint) repaint();
}
