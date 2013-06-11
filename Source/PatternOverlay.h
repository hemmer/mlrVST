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

class PatternOverlay : public Component
{

public:

	PatternOverlay(const int &patternID,
				   mlrVSTAudioProcessor * const owner,
				   PatternRecording * const patternLink,
				   const int &h,
				   const int &w);

	~PatternOverlay() { }

	void paint(Graphics& g);

private:

	const int patternID;
	mlrVSTAudioProcessor * const processor;
	PatternRecording * const patternData;


	//const int height, width;
	Rectangle<int> overlayPaintBounds;

	JUCE_LEAK_DETECTOR(PatternOverlay);
	
};



#endif  // __PATTERNOVERLAY_H_93085C1D__
