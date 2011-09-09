/*
  ==============================================================================

    WaveformControl.h
    Created: 6 Sep 2011 12:38:13pm
    Author:  Hemmer

    Custom component to display a waveform (corresponding to an mlr row)

  ==============================================================================
*/

#ifndef __WAVEFORMCONTROL_H_E96F19F8__
#define __WAVEFORMCONTROL_H_E96F19F8__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"


class WaveformControl  : public Component,
                         public ChangeListener,
                         public ButtonListener,
                         public FileDragAndDropTarget
{
public:
    WaveformControl(const int &id);
	~WaveformControl();

    void setFile (const File& file);
	void setZoomFactor (double amount);
	void mouseWheelMove (const MouseEvent&, float wheelIncrementX, float wheelIncrementY);
    void mouseDown(const MouseEvent&);

    void paint (Graphics& g);
    void changeListenerCallback (ChangeBroadcaster*);
    void buttonClicked(Button *btn);

    bool isInterestedInFileDrag (const StringArray& /*files*/);
    void filesDropped (const StringArray& files, int /*x*/, int /*y*/);

    // if the number of channels changes, we can update the strips
    void addChannel(const int &id, const Colour &col);
    void clearChannelList();
    //void updateChannelColour(const Colour &col);

private:

    // which strip we are representing
    int waveformID;

    // GUI components
    Label trackNumberLbl, filenameLbl;
    OwnedArray<DrawableButton> channelButtonArray;

    // which channel audio is currently going to
    int currentChannel, numChannels;
    
    // stuff for drawing waveforms
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    double startTime, endTime;

    // main strip background colour
    Colour backgroundColour;

    File currentFile;
};



#endif  // __WAVEFORMCONTROL_H_E96F19F8__
