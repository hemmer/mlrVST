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
    visualSelectionStart(0), visualSelectionEnd(0)
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
            backgroundColour = channelButtonArray[i]->getBackgroundColour();
            backgroundColourDark = backgroundColour.darker().darker();
            currentChannel = i;
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
                    demoPage->updateSampleStripSelection(0.0, 1.0, sampleStripID);
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

        // try to send the new selection to the SampleStrips
        mlrVSTAudioProcessorEditor *demoPage = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);

        if (demoPage != 0)
        {
            demoPage->updateSampleStripSelection(visualSelectionStart / (float) componentWidth,
                                                 visualSelectionEnd / (float) componentWidth,
                                                 sampleStripID);
        }

        repaint();
    }
}

void SampleStripControl::mouseDrag(const MouseEvent &e)
{
    visualSelectionEnd = e.x;
    repaint();
    //DBG(e.g
}


void SampleStripControl::paint(Graphics& g)
{
    g.fillAll(backgroundColourDark);
    g.setColour(backgroundColour);

    // this is because we can drag backwards!
    if (visualSelectionEnd > visualSelectionStart)
        g.fillRect(visualSelectionStart, 15, (visualSelectionEnd - visualSelectionStart), componentHeight - 15);
    else g.fillRect(visualSelectionEnd, 15, (visualSelectionStart - visualSelectionEnd), componentHeight - 15);

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

void SampleStripControl::updateThumbnail(File &newFile)
{
    thumbnail.setSource(new FileInputSource(newFile));
    thumbnailLength = thumbnail.getTotalLength();

    // update filename label
    filenameLbl.setText(newFile.getFullPathName(), false);

    DBG("Waveform strip " + String(sampleStripID) + ": file \"" + newFile.getFileName() + "\" selected.");
}

//void SampleStripControl::setAudioSample(AudioSample* sample)
//{
//    // this is now the new audio sample
//    currentSample = sample;
//
//    // by default, the whole sample is selected
//    // TODO: check math of rounding here
//    selectionStart = 0;
//    selectionEnd = currentSample->getSampleLength();
//    selectionStartVisual = 0;
//    selectionEndVisual = componentWidth;
//    chunkSize = (int) ((selectionEnd - selectionStart) / numChunks);
//
//    // get temporary reference to the File object of the
//    // sample to update labels / thumbnails
//    File sampleFile = currentSample->getSampleFile();
//    thumbnail.setSource(new FileInputSource (sampleFile));
//    thumbnailLength = thumbnail.getTotalLength();
//
//    // update filename label
//    filenameLbl.setText(sampleFile.getFullPathName(), false);
//
//    DBG("Waveform strip " + String(waveformID) + ": file \"" + sampleFile.getFileName() + "\" selected.");
//}