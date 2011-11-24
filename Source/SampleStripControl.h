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
#include "mlrVSTLookAndFeel.h"

/* Forward declaration to set up pointer arrangement 
   to allow sample strips to access the UI */
class mlrVSTAudioProcessorEditor;

class SampleStripControl :  public Component,
    public ChangeListener,
    public ButtonListener,
    public ComboBoxListener,
    public SliderListener,
    public FileDragAndDropTarget
{
public:
    SampleStripControl(const int &id,
                       const int &height,
                       const int &width,
                       const int &newNumChannels,
                       SampleStrip * const dataStrip,
                       mlrVSTAudioProcessorEditor * const owner);
    ~SampleStripControl();

    void mouseDown(const MouseEvent&);
    void mouseDrag(const MouseEvent&);
    void mouseUp(const MouseEvent&);

    void paint(Graphics& g);
    void changeListenerCallback(ChangeBroadcaster*);
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged);
    void buttonClicked(Button *btn);
    void sliderValueChanged(Slider *sldr);

    // This allows us to load samples by Drag n' Drop
    bool isInterestedInFileDrag(const StringArray&) { return true; }
    void filesDropped(const StringArray& files, int x, int y);

    /* Picks a new sample from the sample pool,
       selects it all and finds the new playSpeed.
    */
    void selectNewSample(const int &fileChoice);

    // Update the strips if the number of channels changes
    void buildUI();
    void updateChannelColours(const int &newChannel);
    void setNumChannels(const int &newNumChannels) {
        numChannels = newNumChannels;
        buildUI();  // reflect changes
    }

    void updatePlaybackStatus()
    {
        // no need to update if nothing is happening
        if (isPlaying)
        {
            playbackPercentage = *static_cast<const float*>(dataStrip->getSampleStripParam(SampleStrip::pPlaybackPercentage));
            repaint();
        }

        
        isPlaying = *static_cast<const bool*>(dataStrip->getSampleStripParam(SampleStrip::pIsPlaying));

    }

    // These recall settings when the 
    void recallParam(const int &paramID, const void *newValue, const bool &doRepaint);


    void updateThumbnail(const File &newFile);
    void buildNumBlocksList(const int &newMaxNumBlocks);

private:

    // Pointer to parent GUI component
    mlrVSTAudioProcessorEditor * const mlrVSTEditor;
    // which strip we are representing
    const int sampleStripID;
    // Pointer to data structure for samplestrip
    SampleStrip * const dataStrip;

    // GUI components
    Label trackNumberLbl, filenameLbl, chanLbl, volLbl, modeLbl, playSpeedLbl;
    OwnedArray<DrawableButton> channelButtonArray;
    ComboBox selNumChunks, selPlayMode;
    ToggleButton isLatchedBtn, isReversedBtn;
    TextButton times2, div2;
    
    DrawableButton speedLockBtn;
    DrawableImage lockImg, unlockImg;
    bool isSpeedLocked;

    Slider stripVolumeSldr, playbackSpeedSldr;

    const int componentHeight, componentWidth;
    const int controlbarSize;
    Rectangle<int> waveformPaintBounds;


    menuLookandFeel menuLF;

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
    double thumbnailLength, thumbnailScaleFactor;
    const AudioSample *currentSample;

    // main strip background colour
    Colour backgroundColour;

    JUCE_LEAK_DETECTOR(SampleStripControl);  

    const float fontSize;
};



#endif  // __SAMPLESTRIPCONTROL_H_E96F19F8__
