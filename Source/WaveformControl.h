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
#include "ChannelStrip.h"

//class ChannelStrip;

class WaveformControl  : public Component,
                         public ChangeListener,
                         public ButtonListener,
                         public FileDragAndDropTarget
{
public:
    WaveformControl(const int &id, const Array<ChannelStrip> &channelArray);
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
    void updateChannelList(const Array<ChannelStrip> &channelArray);

private:

    Label *trackNumberLbl, *filenameLbl;
    Array<DrawableButton*> channelButtonArray;

    // number of output channels available
    //const int numChannels;
    // which channel audio is currently going to
    int currentChannel;
    // store channels information
    Array<ChannelStrip> channelArray;

    // which strip we are representing
    int waveformID;

    // stuff for drawing waveforms
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    double startTime, endTime;
    // main background colour
    Colour backgroundColour;

    File currentFile;
};



#endif  // __WAVEFORMCONTROL_H_E96F19F8__
