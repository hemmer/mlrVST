/*
==============================================================================

SampleStripControl.cpp
Created: 6 Sep 2011 12:38:13pm
Author:  Hemmer

Custom component to display a waveform (corresponding to an mlr row)

==============================================================================
*/

#include "SampleStripControl.h"
#include "PluginEditor.h"

SampleStripControl::SampleStripControl(const int &id, const int &width, const int &height) :
    componentHeight(height), componentWidth(width),
    waveformPaintBounds(0, 15, componentWidth, componentHeight - 15),
    thumbnailCache(5),
    thumbnail(512, formatManager, thumbnailCache),
    backgroundColour(Colours::black),
    trackNumberLbl("track number", String(sampleStripID)),
    filenameLbl("filename", "No File"),
    sampleStripID(id),
    currentChannel(0), numChannels(0),
    channelButtonArray(),
    isReversed(false),
    numChunks(8),
    thumbnailLength(0.0),
    visualSelectionStart(0), visualSelectionEnd(0), visualChunkSize(0.0), visualSelectionLength(0),
    selNumChunks()
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
}

void SampleStripControl::buildNumBlocksList(const int &newMaxNumBlocks)
{
    selNumChunks.clear();
    for (int i = 1; i <= newMaxNumBlocks; ++i)
        selNumChunks.addItem(String(i), i);

    // select the max number of blocks (comboBoxChanged will 
    // be informed).
    selNumChunks.setSelectedId(newMaxNumBlocks);
}

void SampleStripControl::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    // get pointer to parent class
    mlrVSTAudioProcessorEditor* pluginUI = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);


    if (comboBoxThatHasChanged == &selNumChunks)
    {
        numChunks = selNumChunks.getSelectedId();
        visualChunkSize = (visualSelectionLength / (float) numChunks);
        pluginUI->updateSampleStripParameter(SampleStrip::ParamNumChunks,
                                             numChunks,
                                             sampleStripID);
        repaint();
    }
}

SampleStripControl::~SampleStripControl()
{
    thumbnail.removeChangeListener(this);
}

void SampleStripControl::buttonClicked(Button *btn)
{
    // get pointer to parent class
    mlrVSTAudioProcessorEditor* pluginUI = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);

    // see if any of the channel selection buttons were chosen
    for (int i = 0; i < channelButtonArray.size(); ++i)
    {
        if (channelButtonArray[i] == btn)
        {
            backgroundColour = channelButtonArray[i]->getBackgroundColour();
            backgroundColourDark = backgroundColour.darker().darker();
            currentChannel = i;
            pluginUI->updateSampleStripParameter(SampleStrip::ParamCurrentChannel, currentChannel, sampleStripID);
            repaint();
        }
    }
}

void SampleStripControl::clearChannelList()
{
    // if there are existing buttons, remove them
    if (channelButtonArray.size() != 0) channelButtonArray.clear();
    numChannels = 0;
}


/* Used to (re)add Buttons to control the switching channels.
   Useful if we want to change the total number of channels at runtime. */
void SampleStripControl::addChannel(const int &id, const Colour &col)
{

    channelButtonArray.add(new DrawableButton("button" + String(numChannels), DrawableButton::ImageRaw));
    addAndMakeVisible(channelButtonArray.getLast());
    channelButtonArray.getLast()->setBounds(15 + numChannels * 15, 0, 15, 15);
    channelButtonArray.getLast()->addListener(this);
    channelButtonArray.getLast()->setBackgroundColours(col, col);


    // once we've added all the buttons, add the filename
    // TODO NOW: filenameLbl.setBounds((i+1)*15, 0, 500, 15);

    // reset channel to first channel just in case
    // old channel setting no longer exists
    currentChannel = 0;
    backgroundColour = channelButtonArray.getFirst()->getBackgroundColour();
    backgroundColourDark = backgroundColour.darker().darker();
    // let the UI know
    repaint();

    ++numChannels;

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

    if (e.mods == ModifierKeys::rightButtonModifier)
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
                    
                    demoPage->updateSampleStripSelection(0.0, 1.0, sampleStripID);

                    repaint();
                }
            }
        }
    }
    else
    {
        visualSelectionStart = e.getMouseDownX();
        visualSelectionEnd = visualSelectionStart;
    }
}

void SampleStripControl::mouseUp(const MouseEvent &e)
{
    if (e.mods == ModifierKeys::leftButtonModifier)
    {

        // we might have selected backwards
        if (e.x > visualSelectionStart) visualSelectionEnd = e.x;
        else
        {
            visualSelectionEnd = visualSelectionStart;
            visualSelectionStart = e.x;
        }
        // Make sure we don't select outside the waveform
        if (visualSelectionEnd > componentWidth) visualSelectionEnd = componentWidth;
        if (visualSelectionStart < 0) visualSelectionStart = 0;

        visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
        visualChunkSize = (visualSelectionLength / (float) numChunks);

        // try to send the new selection to the SampleStrips
        mlrVSTAudioProcessorEditor *demoPage = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);

        // let the sample strip know about the change
        if (demoPage != 0)
        {
            demoPage->updateSampleStripSelection(visualSelectionStart / (float) componentWidth,
                                                 visualSelectionEnd / (float) componentWidth,
                                                 sampleStripID);
        }

        // redraw to reflect new selection
        repaint();
    }
}

void SampleStripControl::mouseDrag(const MouseEvent &e)
{
    visualSelectionEnd = e.x;

    // Make sure we don't select outside the waveform
    if (visualSelectionEnd > componentWidth) visualSelectionEnd = componentWidth;
    if (visualSelectionEnd < 0) visualSelectionEnd = 0;

    if (visualSelectionEnd > visualSelectionStart)
        visualSelectionLength = (visualSelectionEnd - visualSelectionStart);
    else
        visualSelectionLength = (visualSelectionStart - visualSelectionEnd);

    visualChunkSize = (visualSelectionLength / (float) numChunks);
    repaint();
}


void SampleStripControl::paint(Graphics& g)
{
    g.fillAll(backgroundColourDark);
    g.setColour(backgroundColour);

    // this may seem complicated but we need to be able to
    // paint the strip even if the selection is made backwards!
    if (visualSelectionEnd > visualSelectionStart)
    {
        for(int c = 0; c < numChunks; ++c)
        {
            Rectangle<float> chunkC((float) (visualSelectionStart + c * visualChunkSize),
                                    15.0f, visualChunkSize, (float) (componentHeight - 15.0f));
            g.fillRoundedRectangle(chunkC, 8.0f);
        }
    }
    else
    {
        for(int c = 0; c < numChunks; ++c)
        {
            Rectangle<float> chunkC((float) (visualSelectionEnd + c * (visualChunkSize)),
                                    15.0f, visualChunkSize, (float) (componentHeight - 15.0f));
            g.fillRoundedRectangle(chunkC, 8.0f);
        }
    }
        
    // TODO: grey out waveform outside selection
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

    if (isPlaying)
    {
        g.setColour(Colours::black);
        g.drawFittedText("Playback at " + String(playbackPercentage), 0, 15, componentWidth, componentHeight - 15, Justification::centred, 2);
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
