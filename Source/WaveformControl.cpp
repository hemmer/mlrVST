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
        backgroundColour(Colours::black),
        trackNumberLbl(0),
        filenameLbl(0),
        waveformID(id),
        currentFile(),
        //numChannels(channelArray.size()),
        currentChannel(0),
        channelButtonArray(),
        channelArray()
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

    addAndMakeVisible(filenameLbl = new Label("filename", "No File"));
    filenameLbl->setColour(Label::backgroundColourId, Colours::white);
    filenameLbl->setColour(Label::textColourId, Colours::black);
    filenameLbl->setFont(10.0f);

    updateChannelList(chanArray);
}

WaveformControl::~WaveformControl()
{
    thumbnail.removeChangeListener (this);
    deleteAndZero(trackNumberLbl);
    deleteAndZero(filenameLbl);

    for(int i = 0; i < channelButtonArray.size(); ++i)
    {
        DrawableButton *tempBtn = channelButtonArray[i];
        deleteAndZero(tempBtn);
    }
}

// update the thumbnail to reflect a new choice of file
void WaveformControl::setFile (const File& file)
{
    thumbnail.setSource(new FileInputSource (file));
    startTime = 0;
    endTime = thumbnail.getTotalLength();

    // update filename label
    filenameLbl->setText(file.getFileName(), false);
    currentFile = File(file);
}

void WaveformControl::buttonClicked(Button *btn){
    for(int i = 0; i < channelButtonArray.size(); ++i){
        if(channelButtonArray[i] == btn){
            backgroundColour = channelArray[i].getColour();
            currentChannel = i;
            repaint();
        }
    }
}


// Used to (re)add Buttons to control the switching channels.
// Useful if we want to change the total number of channels at runtime.
void WaveformControl::updateChannelList(const Array<ChannelStrip> &chanArray)
{
    channelArray = Array<ChannelStrip>(chanArray);
    int i = 0;  // for interating over loops

    // sanity check: make sure we actually
    // have an array to work with
    if(channelArray.size() != 0)
    {
        // if there are existing buttons, remove them first
        // as total number of channels may have decreased
        if(channelButtonArray.size() != 0){
            // TODO: check is deleteAndZero really necessary?
            for(i = 0; i < channelButtonArray.size(); ++i)
            {
                DrawableButton *tempBtn = channelButtonArray[i];
                deleteAndZero(tempBtn);
            }
            channelButtonArray.clear();
        }

        // (re)add the updated channel spec
        for(i = 0; i < channelArray.size(); ++i)
        {
            DrawableButton *tempBtn = new DrawableButton("button" + String(i), DrawableButton::ImageRaw);
            channelButtonArray.add(tempBtn);
            addAndMakeVisible(channelButtonArray[i]);
            channelButtonArray[i]->setBounds(15 + i*15, 0, 15, 15);
            channelButtonArray[i]->addListener(this);
            channelButtonArray[i]->setBackgroundColours(channelArray[i].getColour(),
                                                        channelArray[i].getColour());
        }

        filenameLbl->setBounds((i+1)*15, 0, 500, 15);

        // reset channel to first channel just in case 
        // old channel setting no longer exists
        currentChannel = 0;
        backgroundColour = channelArray[currentChannel].getColour();
        // let the UI know
        repaint();
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

            // only show the menu if we have files to show
            if(loadedFiles.size() != 0)
            {
                PopupMenu p = PopupMenu();
            
                // TODO: add option to select "no file
                // TODO: middle click to delete sample under cursor in menu?
                for(int i = 0; i < loadedFiles.size(); ++i)
                {
                    // +1 is because 0 is result for no item clicked
                    p.addItem(i+1, String(loadedFiles[i].getFileName()));
                }

                // show the menu and store choice 
                int fileChoice = p.showMenu(PopupMenu::Options().withTargetComponent(filenameLbl));

                if (fileChoice != 0)
                {
                    --fileChoice;       // -1 is to correct for +1 in for loop above
                    demoPage->recieveFileSelection(waveformID, fileChoice);
                    DBG("Waveform strip" + String(waveformID) + ": file selected " + loadedFiles[fileChoice].getFileName());
                }
            }
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

void WaveformControl::filesDropped (const StringArray& /*files*/, int /*x*/, int /*y*/)
{
    //mlrVSTAudioProcessorEditor* demoPage = findParentComponentOfClass ((mlrVSTAudioProcessorEditor*) 0);

    //if (demoPage != 0)
        //demoPage->helloLabel.setText("boom");
//        showFile (File (files[0]));
}

