/*
  ==============================================================================

    PatternOverlay.cpp
    Created: 11 Jun 2013 10:08:26pm
    Author:  hemmer

  ==============================================================================
*/

#include "PatternStripControl.h"

PatternStripControl::PatternStripControl(const int &patternID,
										 mlrVSTAudioProcessor * const owner,
										 PatternRecording * const patternLink,
										 const int &w, const int &h) :

	patternID(patternID), processor(owner),
	patternData(patternLink),
	overlayPaintBounds(w, h),
	upperHalf(0, 0, w, h/2), lowerHalf(0, h/2, w, h/2)
{

}



void PatternStripControl::paint(Graphics& g)
{
    // Start with the background colour
	g.setColour(Colours::black.withAlpha(0.6f));
	g.fillRect(upperHalf);

	g.setColour(Colours::white);
	g.fillRect(lowerHalf);

	g.setFont(Font("ProggyCleanTT", 18.f, Font::plain));

	g.drawText("pattern " + String(patternID), 5, 5, 250, 10, Justification::centredLeft, true );
	g.drawText("length " + String(patternData->patternLength), 5, 20, 250, 10, Justification::centredLeft, true );

	MidiBuffer::Iterator it(patternData->midiPattern);
	MidiMessage result;
	const int lengthInSamples = patternData->patternLengthInSamples;
	int samplePosition;

	int defaultVal = 0;
	Array<int> sampleOn(&defaultVal, 16);


	const int halfHeight = overlayPaintBounds.getHeight() / 2;
	const int width = overlayPaintBounds.getWidth();

	while(it.getNextEvent(result, samplePosition))
	{
		const int chan = result.getChannel();

		g.setColour(Colour(chan/8.0f, 0.5f, 0.5f, 1.0f));


		if (result.isNoteOn())
		{
			sampleOn.set(chan, samplePosition);
		}
		else if (result.isNoteOff())
		{
			const int diff = samplePosition - sampleOn[chan];

			if (diff < 0) continue;

			DBG(sampleOn[chan] << " " << samplePosition << "pattern " << lengthInSamples << " " << diff);
			const int startX = width * ((double) sampleOn[chan] / (double) lengthInSamples);
			const int noteWidth = width * (double)(diff) / (double) lengthInSamples;
			const int startY = ((double) chan / 8.0) * halfHeight + halfHeight;
			const int noteHeight = ((double) halfHeight / 8.0);

			DBG("chan: " << chan << " " << startX << " " << startY << " " << noteWidth);

			g.fillRect(startX, startY, noteWidth, noteHeight);

			// set one past the end
			sampleOn.set(chan, lengthInSamples + 1);
		}
	}

	float playbackPosX = width * patternData->getPatternPercent();
	
	g.drawLine(playbackPosX, 0, playbackPosX, overlayPaintBounds.getHeight(), 1.0f);
}
