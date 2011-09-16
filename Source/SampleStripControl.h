/*
  ==============================================================================

    SampleStripControl.h
    Created: 6 Sep 2011 12:38:13pm
    Author:  Hemmer

    Custom component to display a waveform (corresponding to an mlr row)

  ==============================================================================
*/

#ifndef __SAMPLESTRIPCONTROL_H_E96F19F8__
#define __SAMPLESTRIPCONTROL_H_E96F19F8__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"
#include "AudioSample.h"
#include "SampleStripControl.h"

class SampleStripControl :  public Component,
    public ChangeListener,
    public ButtonListener,
    public ComboBoxListener,
    public FileDragAndDropTarget
{
public:
    SampleStripControl(const int &id, const int &height, const int &width, const int &newNumChannels);
    ~SampleStripControl();

    //void setFile (const File& file);
    void setZoomFactor(double amount);
    void mouseWheelMove(const MouseEvent&, float wheelIncrementX, float wheelIncrementY);
    void mouseDown(const MouseEvent&);
    void mouseDrag(const MouseEvent&);
    void mouseUp(const MouseEvent&);

    void paint(Graphics& g);
    void changeListenerCallback(ChangeBroadcaster*);
    void buttonClicked(Button *btn);
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged);

    // This allows us to load samples by Drag n' Drop
    bool isInterestedInFileDrag(const StringArray& files);
    void filesDropped(const StringArray& files, int x, int y);

    // Update the strips if the number of channels changes
    void buildChannelButtonList(const int &newNumChannels);
    void setChannel(const int &newChannel);

    // These recall settings when the 
    void recallParam(const int &paramID, const void *newValue, const bool &doRepaint);


    void updateThumbnail(const File &newFile);
    void buildNumBlocksList(const int &newMaxNumBlocks);

private:

    ComboBox selNumChunks, selPlayMode;
    ToggleButton isLatchedBtn, isReversedBtn;

    const int componentHeight, componentWidth;
    Rectangle<int> waveformPaintBounds;

    // which strip we are representing
    int sampleStripID;

    // GUI components
    Label trackNumberLbl, filenameLbl;
    OwnedArray<DrawableButton> channelButtonArray;

    int numChannels;        // total number of channels available

    /* ============================================================
       properties about the waveformcontrols handling of the sample
       note these are waveform strip independent, i.e. they are not
       properties of the AudioSample. This means we can have
       different start points on different rows for the same sample
       ============================================================*/
    bool isReversed, isPlaying, isLatched;
    float playbackPercentage;

    // these are the same, except refering to the component
    // rather than the sample
    int visualSelectionStart, visualSelectionEnd;
    int visualSelectionLength;
    // this need to be a float as chunkSize might need anti aliasing.
    float visualChunkSize;
    // how many chunks to break the sample into (default 8 for
    // standard monome) and what size (in samples) they are
    int numChunks;

    // This is so we can drag with middle mouse
    int selectionStartBeforeDrag;
    int *selectionPointToChange;
    int dragStart;
    ModifierKeys mouseDownMods;

    // stuff for drawing waveforms
    // TODO: should this be associated with the AudioSample?
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    double thumbnailLength;

    // main strip background colour
    Colour backgroundColour;
};



#endif  // __SAMPLESTRIPCONTROL_H_E96F19F8__
