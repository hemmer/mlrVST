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
#include "AudioSample.h"
#include "SampleStripControl.h"
#include "mlrVSTLookAndFeel.h"
#include "CustomArrowButton.h"
#include "TextDragSlider.h"

/* Forward declaration to set up pointer arrangement
   to allow sample strips to access the UI */
class mlrVSTGUI;

class SampleStripControl :  public Component,
    public ChangeListener,
    public ButtonListener,
    public ComboBoxListener,
    public FileDragAndDropTarget
{
public:
    SampleStripControl(const int &id,
                       const int &height,
                       const int &width,
                       const int &newNumChannels,
                       SampleStrip * const dataStrip,
                       mlrVSTAudioProcessor * const owner);
    ~SampleStripControl();

    enum hitZone
    {
        pNone,
        pSample,
        pRecord,
        pResample
    };

    void mouseDown(const MouseEvent&);
    void mouseDrag(const MouseEvent&);
    void mouseUp(const MouseEvent&);

    void paint(Graphics& g);
    void changeListenerCallback(ChangeBroadcaster*);
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged);
    void buttonClicked(Button *btn);

    // This allows us to load samples by Drag n' Drop
    bool isInterestedInFileDrag(const StringArray&) { return true; }
    void filesDropped(const StringArray& files, int x, int y);

    // Picks a new sample from the sample pool,
    // selects it all and finds the new playSpeed.
    void selectNewSample(const int &fileChoice, const int &poolID);

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

    void updateParamsIfChanged()
    {
        if (stripChanged)
        {
            for(int p = SampleStrip::FirstParam; p < SampleStrip::NumGUIParams; ++p)
            {
                const void *newValue = dataStrip->getSampleStripParam(p);
                recallParam(p, newValue, false);
            }

            // repaint once all params are checked
            repaint();
            stripChanged = false;
        }
    }

    // These recall settings and update the GUI appropriately
    void recallParam(const int &paramID, const void *newValue, const bool &doRepaint);

    void updateThumbnail(const File &newFile);

private:

    // ID / communication //////////////////////////
    mlrVSTAudioProcessor * const processor;
    const int sampleStripID;            // which strip we are representing
    SampleStrip * const dataStrip;      // pointer to data structure for samplestrip
    bool stripChanged;                  // do we need to redraw

    // GUI dimensions //////////////////////////////
    const int componentHeight, componentWidth;
    const int controlbarSize;
    const int maxWaveformHeight;
    Rectangle<int> waveformPaintBounds;

    // SampleStrip GUI ////////////////////////////////////////
	overrideLookandFeel overrideLF;
	Font defaultFont;
    Colour backgroundColour;
    // channel selector
    Label chanLbl;
    OwnedArray<DrawableButton> channelButtonArray;
    // volume controls
    Label volLbl;
    TextDragSlider stripVolumeSldr;
    // playmode selector
    Label modeLbl;
    ComboBox selPlayMode;
    ToggleButton isLatchedBtn;
    // playspeed controls
    Label playspeedLbl;
    TextDragSlider playspeedSldr;
    DrawableButton speedLockBtn;
    CustomArrowButton isReversedBtn;
    DrawableImage lockImg, unlockImg;
    TextButton times2, div2;
    // select num chunks
	Label numChunksLabel;
    TextDragSlider selNumChunks;
    // bottom row labels
    Label trackNumberLbl, filenameLbl;
    // bit of a hack, for some reason popup menu needs a component
    // to spawn from so I use blank labels
    OwnedArray<Label> popupLocators;

    // Waveform control ////////////////////////
    // dimensions of selection in pixels
    int visualSelectionStart, visualSelectionEnd, visualSelectionLength;
    float visualChunkSize;      // this needs to be a float as chunkSize might need anti-aliasing.
    int numChunks;              // how many chunks to break the sample into
    // This is so we can drag with middle mouse
    int selectionStartBeforeDrag, *selectionPointToChange, *selectionPointFixed;
    ModifierKeys mouseDownMods;         // so we can track what mouse combination is used
    bool rightMouseDown;                // track RMB usage
    int selectedHitZone;                // what type of sample are we selecting
    double thumbnailScaleFactor;        // scale waveform height by volume
    const AudioSample *currentSample;   // pointer to the sample (to get waveform)

    // Settings //////////////
    int numChannels;                // total number of channels available
    bool isSpeedLocked;             // does playspeed change with selection
    bool isLatched;                 // if latched then button up events are ignored
    bool isReversed, isPlaying;     // play settings
    float playbackPercentage;

    JUCE_LEAK_DETECTOR(SampleStripControl);
};



#endif  // __SAMPLESTRIPCONTROL_H_E96F19F8__
