/*
  ==============================================================================

    WaveformControl.h
    Created: 6 Sep 2011 12:38:13pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __WAVEFORMCONTROL_H_E96F19F8__
#define __WAVEFORMCONTROL_H_E96F19F8__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"


class WaveformControl  : public Component,
                           public ChangeListener,
                           public FileDragAndDropTarget
{
public:
    WaveformControl(const Colour&);
	~WaveformControl();

    void setFile (const File& file);
	void setZoomFactor (double amount);
	void mouseWheelMove (const MouseEvent&, float wheelIncrementX, float wheelIncrementY);

    void paint (Graphics& g);
    void changeListenerCallback (ChangeBroadcaster*);

    bool isInterestedInFileDrag (const StringArray& /*files*/);
    void filesDropped (const StringArray& files, int /*x*/, int /*y*/);

    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    double startTime, endTime;

private:
	Colour backgroundColour;
};



#endif  // __WAVEFORMCONTROL_H_E96F19F8__
