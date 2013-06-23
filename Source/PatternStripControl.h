/*
==============================================================================

PatternOverlay.h
Created: 11 Jun 2013 10:08:26pm
Author:  hemmer

==============================================================================
*/

#ifndef __PATTERNOVERLAY_H_93085C1D__
#define __PATTERNOVERLAY_H_93085C1D__

#include "../JuceLibraryCode/JuceHeader.h"
#include "mlrVSTGUI.h"

class PatternStripControl : public Component,
    public ChangeListener
{

public:

    PatternStripControl(const int &patternID,
        mlrVSTAudioProcessor * const owner,
        PatternRecording * const patternLink,
        const int &h, const int &w);

    ~PatternStripControl();

    void changeListenerCallback(ChangeBroadcaster * sender);

    void paint(Graphics& g);

private:

    const int patternID;
    mlrVSTAudioProcessor * const processor;
    PatternRecording * const patternData;

    Rectangle<int> overlayPaintBounds;
    Rectangle<int> upperHalf, lowerHalf;

    // Store a visual copy of the Pattern so it doesn't
    // need to be redrawn every call to paint()
    void cachePattern();

    // Cache note positions and colours
    Array<Rectangle<float> > notePositions;
    Array<Colour> noteColours;

    JUCE_LEAK_DETECTOR(PatternStripControl);

};

#endif  // __PATTERNOVERLAY_H_93085C1D__
