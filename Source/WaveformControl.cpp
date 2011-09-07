/*
==============================================================================

WaveformControl.cpp
Created: 6 Sep 2011 12:38:13pm
Author:  Hemmer

Custom component to display a waveform (corresponding to an mlr row)

==============================================================================
*/

#include "WaveformControl.h"
#include "PluginEditor.h"

WaveformControl::WaveformControl(const int &id, const Array<ChannelStrip> &chanArray) :
    	thumbnailCache(5),
        thumbnail(512, formatManager, thumbnailCache),
        backgroundColour(channelsArray[currentChannel].getColour()),
        trackNumberLbl(0),
        waveformID(id),
        currentFile(),
        //numChannels(channelsArray.size()),
        currentChannel(0),
        channelButtonArray(),
        channelsArray(chanArray)
{
    Rectangle<int> waveformShape = getBoundsInParent();

    startTime = endTime = 0;
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener (this);

    addAndMakeVisible(trackNumberLbl = new Label("track number", String(waveformID)));
    trackNumberLbl->setBounds(0, 0, 15, 15);
    trackNumberLbl->setColour(Label::backgroundColourId, Colours::black);
    trackNumberLbl->setColour(Label::textColourId, Colours::white);
    trackNumberLbl->setFont(10.0f);

    jassert(channelsArray.size() == 4)

    for(int i = 0; i < channelsArray.size(); ++i)
    {
        DrawableButton *tempBtn = new DrawableButton("button" + String(i), DrawableButton::ImageRaw);
        channelButtonArray.add(tempBtn);
        addAndMakeVisible(channelButtonArray[i]);
        channelButtonArray[i]->setBounds(50 + i*15, 0, 15, 10);
        channelButtonArray[i]->addListener(this);
        channelButtonArray[i]->setBackgroundColours(channelsArray[i].getColour(),
                                                    channelsArray[i].getColour());
    }
}

WaveformControl::~WaveformControl()
{
    thumbnail.removeChangeListener (this);
    deleteAndZero(trackNumberLbl);
}

// update the thumbnail to reflect a new choice of file
void WaveformControl::setFile (const File& file)
{
    thumbnail.setSource(new FileInputSource (file));
    startTime = 0;
    endTime = thumbnail.getTotalLength();

    // update filename label
    currentFile = File(file);
}

void WaveformControl::buttonClicked(Button *btn){
    for(int i = 0; i < channelButtonArray.size(); ++i){
        if(channelButtonArray[i] == btn){
            backgroundColour = channelsArray[i].getColour();
            currentChannel = i;
            repaint();
        }
    }
}

void WaveformControl::setZoomFactor (double amount)
{
    if (thumbnail.getTotalLength() > 0)
    {
        double timeDisplayed = jmax (0.001, (thumbnail.getTotalLength() - startTime) * (1.0 - jlimit (0.0, 1.0, amount)));
        endTime = startTime + timeDisplayed;
        repaint();
    }
}

void WaveformControl::mouseWheelMove (const MouseEvent&, float wheelIncrementX, float wheelIncrementY)
{
    if (thumbnail.getTotalLength() > 0)
    {
        double newStart = startTime + (wheelIncrementX + wheelIncrementY) * (endTime - startTime) / 10.0;
        newStart = jlimit (0.0, thumbnail.getTotalLength() - (endTime - startTime), newStart);
        endTime = newStart + (endTime - startTime);
        startTime = newStart;
        repaint();
    }
}

void WaveformControl::mouseDown(const MouseEvent &e){

    if(e.mods == ModifierKeys::rightButtonModifier){

        mlrVSTAudioProcessorEditor *demoPage = findParentComponentOfClass((mlrVSTAudioProcessorEditor*) 0);
        
        if(demoPage != 0){
            Array<File> loadedFiles = demoPage->getLoadedFiles();

            PopupMenu p = PopupMenu();

            // TODO: if no popup item selected, 0 is returned selecting
            // the first file in the list

            // TODO: add option to select "none"

            // TODO: right click to delete sample under cursor
            for(int i = 0; i < loadedFiles.size(); ++i)
            {
                p.addItem(i, String(loadedFiles[i].getFileName()));
            }
            int fileChoice = p.show();
            demoPage->recieveFileSelection(waveformID, fileChoice);
            demoPage->debugMe(String(channelsArray[1].getChannelNum()));
        }

        // DO WE STORE A FILE OBJECT IN THE WAVEFORMCONTROL?
        // OR KEEP IT IN THE PARENT

        //trackInfo->setText(labeltext, false);


    }
}

void WaveformControl::paint(Graphics& g)
{
	g.fillAll (backgroundColour);
    g.setColour (Colours::white);

    if (thumbnail.getTotalLength() > 0)
    {
        g.setColour (Colours::black);
        thumbnail.drawChannel(g, getLocalBounds().reduced(2, 2),
                              startTime, endTime, 0, 1.0f);

        g.setColour (Colours::white);
        
        thumbnail.drawChannel(g, getLocalBounds().reduced(2, 2),
                              startTime, endTime, 1, 1.0f);

        //thumbnail.drawChannels (g, getLocalBounds().reduced (2, 2),
        //                        startTime, endTime, 1.0f);

        g.setFont (16.0f);
        g.setColour(Colours::black);
        g.drawFittedText (String(currentFile.getFileName()), 0, 0, getWidth(), getHeight(),
                            Justification::centred, 2);
    }
    else
    {
        g.setFont (12.0f);
        g.drawFittedText ("(No audio file selected)", 0, 0, getWidth(), getHeight(),
                            Justification::centred, 2);
    }
}

void WaveformControl::changeListenerCallback (ChangeBroadcaster*)
{
    // this method is called by the thumbnail when it has changed, so we should repaint it..
    repaint();
}

bool WaveformControl::isInterestedInFileDrag (const StringArray& /*files*/)
{
    return true;
}

void WaveformControl::filesDropped (const StringArray& files, int /*x*/, int /*y*/)
{
    //mlrVSTAudioProcessorEditor* demoPage = findParentComponentOfClass ((mlrVSTAudioProcessorEditor*) 0);

    //if (demoPage != 0)
        //demoPage->helloLabel.setText("boom");
//        showFile (File (files[0]));
}

