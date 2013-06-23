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
    upperHalf(0, 0, w, h/2), lowerHalf(0, h/2, w, h/2),
    notePositions(), noteColours()
{
    patternData->addChangeListener(this);
}

PatternStripControl::~PatternStripControl()
{
    notePositions.clear();
    noteColours.clear();
    patternData->removeChangeListener(this);
}

void PatternStripControl::cachePattern()
{
    MidiBuffer::Iterator it(patternData->midiPattern);
    MidiMessage result;

    const int lengthInSamples = patternData->patternLengthInSamples;
    int samplePosition;

    int defaultVal = 0;
    Array<int> sampleOn(&defaultVal, 16);

    const int halfHeight = overlayPaintBounds.getHeight() / 2;
    const int width = overlayPaintBounds.getWidth();

    noteColours.clear(); notePositions.clear();

    // loop through every MIDI event
    while(it.getNextEvent(result, samplePosition))
    {
        const int chan = result.getChannel();

        if (result.isNoteOn())
        {
            sampleOn.set(chan, samplePosition);
        }
        else if (result.isNoteOff())
        {
            // if we have a note-on note-off pair, find the
            // difference between them in samples
            const int diff = samplePosition - sampleOn[chan];

            if (diff < 0) continue;

            const int startX = width * ((double) sampleOn[chan] / (double) lengthInSamples);
            const int noteWidth = width * (double)(diff) / (double) lengthInSamples;
            const int startY = ((double) chan / 8.0) * halfHeight + halfHeight;
            const int noteHeight = ((double) halfHeight / 8.0);

            noteColours.add(Colour((float) chan / 8.0f, 0.5f, 0.7f, 1.0f));
            notePositions.add(Rectangle<float>(startX, startY, noteWidth, noteHeight));

            // set one past the end
            sampleOn.set(chan, lengthInSamples + 1);
        }
    }
}

void PatternStripControl::changeListenerCallback(ChangeBroadcaster * sender)
{
    // TODO: this potentially gets called a lot, maybe use timer?
    if (sender == patternData)
    {
        repaint();

        // store copy of pattern for easy drawing
        cachePattern();
    }
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


    for (int i = 0; i < notePositions.size(); ++i)
    {
        g.setColour(noteColours[i]);
        g.fillRect(notePositions[i]);
    }

    float playbackPosX = overlayPaintBounds.getWidth() * patternData->getPatternPercent();

    g.drawLine(playbackPosX, 0, playbackPosX, overlayPaintBounds.getHeight(), 1.0f);
}