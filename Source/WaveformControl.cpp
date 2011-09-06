/*
==============================================================================

WaveformControl.cpp
Created: 6 Sep 2011 12:38:13pm
Author:  Hemmer

==============================================================================
*/

#include "WaveformControl.h"
#include "PluginEditor.h"

WaveformControl::WaveformControl(const Colour &bg, const int &id) :
    	thumbnailCache(5),
        thumbnail(512, formatManager, thumbnailCache),
		backgroundColour(bg),
        trackInfo(0),
        waveformID(id)
        //demoPage(0)
{
    startTime = endTime = 0;
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener (this);

    addAndMakeVisible(trackInfo = new Label("trackinfo", "hai"));
    trackInfo->setBounds(0, 0, 50, 20);
    trackInfo->setColour(Label::backgroundColourId, Colours::cornflowerblue);

   
}

WaveformControl::~WaveformControl()
{
    thumbnail.removeChangeListener (this);
}

void WaveformControl::setFile (const File& file)
{
    thumbnail.setSource (new FileInputSource (file));
    startTime = 0;
    endTime = thumbnail.getTotalLength();
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


            for(int i = 0; i < loadedFiles.size(); ++i){
                p.addItem(i, String(loadedFiles[i].getFileName()));
            }
            int fileChoice = p.show();
            demoPage->recieveFileSelection(waveformID, fileChoice);
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
        thumbnail.drawChannels (g, getLocalBounds().reduced (2, 2),
                                startTime, endTime, 1.0f);
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

