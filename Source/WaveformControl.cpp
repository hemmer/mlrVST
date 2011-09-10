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

WaveformControl::WaveformControl(const int &id) :
    	thumbnailCache(5),
        thumbnail(512, formatManager, thumbnailCache),
        backgroundColour(Colours::black),
        trackNumberLbl("track number", String(waveformID)),
        filenameLbl("filename", "No File"),
        waveformID(id),
        currentFile(),
        currentChannel(0), numChannels(0),
        channelButtonArray()
        //channelArray()
{
    // TODO: doesn't get bounds until initialised
    Rectangle<int> waveformShape = getBoundsInParent();

    addAndMakeVisible(&filenameLbl);
    filenameLbl.setColour(Label::backgroundColourId, Colours::white);
    filenameLbl.setColour(Label::textColourId, Colours::black);
    filenameLbl.setJustificationType(Justification::right);
    // TODO: make sure this is same as parent
    filenameLbl.setBounds(0, 0, 300, 15);
    filenameLbl.setFont(10.0f);

    startTime = endTime = 0;
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener (this);

    addAndMakeVisible(&trackNumberLbl);
    trackNumberLbl.setBounds(0, 0, 15, 15);
    trackNumberLbl.setColour(Label::backgroundColourId, Colours::black);
    trackNumberLbl.setColour(Label::textColourId, Colours::white);
    trackNumberLbl.setFont(10.0f);



}

WaveformControl::~WaveformControl()
{
    thumbnail.removeChangeListener (this);
}

//void WaveformControl::updateChannelColour(const Colour &col){
//    backgroundColour = col;
//}

// update the thumbnail to reflect a new choice of file
void WaveformControl::setFile (const File& file)
{
    thumbnail.setSource(new FileInputSource (file));
    startTime = 0;
    endTime = thumbnail.getTotalLength();

    // update filename label
    filenameLbl.setText(file.getFileName(), false);
    currentFile = File(file);
}

void WaveformControl::buttonClicked(Button *btn){

    // see if any of the channel selection buttons were chosen
    for(int i = 0; i < channelButtonArray.size(); ++i){
        if(channelButtonArray[i] == btn){
            backgroundColour = channelButtonArray[i]->getBackgroundColour();
            currentChannel = i;
            repaint();
        }
    }
}

void WaveformControl::clearChannelList()
{
    // if there are existing buttons, remove them
    if(channelButtonArray.size() != 0) channelButtonArray.clear();
    numChannels = 0;
}


// Used to (re)add Buttons to control the switching channels.
// Useful if we want to change the total number of channels at runtime.
void WaveformControl::addChannel(const int &id, const Colour &col)
{

    channelButtonArray.add(new DrawableButton("button" + String(numChannels), DrawableButton::ImageRaw));
    addAndMakeVisible(channelButtonArray.getLast());
    channelButtonArray.getLast()->setBounds(15 + numChannels*15, 0, 15, 15);
    channelButtonArray.getLast()->addListener(this);
    channelButtonArray.getLast()->setBackgroundColours(col, col);


    // once we've added all the buttons, add the filename
    // TODO NOW: filenameLbl.setBounds((i+1)*15, 0, 500, 15);

    // reset channel to first channel just in case 
    // old channel setting no longer exists
    currentChannel = 0;
    backgroundColour = channelButtonArray.getFirst()->getBackgroundColour();
    // let the UI know
    repaint();

    ++numChannels;

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
                int fileChoice = p.showMenu(PopupMenu::Options().withTargetComponent(&filenameLbl));

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

