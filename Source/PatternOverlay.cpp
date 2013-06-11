/*
  ==============================================================================

    PatternOverlay.cpp
    Created: 11 Jun 2013 10:08:26pm
    Author:  hemmer

  ==============================================================================
*/

#include "PatternOverlay.h"

PatternOverlay::PatternOverlay(const int &patternID,
							   mlrVSTAudioProcessor * const owner,
							   PatternRecording * const patternLink,
							   const int &w,
							   const int &h) :

	patternID(patternID), processor(owner),
	patternData(patternLink),
	overlayPaintBounds(w, h)

{

}



void PatternOverlay::paint(Graphics& g)
{
    // Start with the background colour
	g.setColour(Colours::black.withAlpha(0.6f));
	g.fillRect(overlayPaintBounds);

	g.setColour(Colours::white);

	g.setFont(Font("Verdana", 16.f, Font::plain));
	g.drawText("pattern " + String(patternID), 5, 5, 250, 10, Justification::centredLeft, true );

	g.setFont(Font("Verdana", 10.f, Font::plain));
	g.drawText("length " + String(patternData->patternLength), 5, 20, 250, 10, Justification::centredLeft, true );


}
