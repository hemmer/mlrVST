/*
==============================================================================

    HintOverlay.h
    Created: 28 June 2013 9:03:08pm
    Author:  Hemmer

==============================================================================
*/

#ifndef __HINTOVERLAY_H_
#define __HINTOVERLAY_H_

#include "../JuceLibraryCode/JuceHeader.h"

class mlrVSTAudioProcessor;


class HintOverlay : public Component, public ChangeListener
{
public:

    HintOverlay(mlrVSTAudioProcessor * const owner);
    ~HintOverlay();

    void changeListenerCallback(ChangeBroadcaster * /*source*/) { repaint(); }

    void paint(Graphics & g);

private:
    // communication //////////////////////
    mlrVSTAudioProcessor * const processor;

    // gui / components ///////////////////
    Rectangle<int> overlayPaintBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HintOverlay);
};


#endif
