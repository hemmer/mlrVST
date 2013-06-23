/*
  ==============================================================================

    PatternRecording.h
    Created: 20 May 2012 12:09:22pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __PATTERNRECORDING_H_E05E291D__
#define __PATTERNRECORDING_H_E05E291D__

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

class PatternRecording : public ChangeBroadcaster
{
public:

    PatternRecording(mlrVSTAudioProcessor* owner, const int &idNumber);

    // rip MIDI messages from host and store them for playback
    void recordPattern(MidiBuffer &midiMessages, const int &numSamples);
    // add the pattern's MIDI messages to the main MIDI buffer for playback
    void playPattern(MidiBuffer &midiMessages, const int &numSamples);

    void startPatternRecording();
    void stopPatternRecording();

    void startPatternPlaying(const int &position = 0);
    void resumePatternPlaying();
    void stopPatternPlaying();

    // these are useful for giving visual feedback
    float getPatternPrecountPercent() const;
    float getPatternPercent() const;

    // which pattern does this object represent
    const int slotID;

    // MIDI ////////////////////
    // this store the main pattern for playback
    MidiBuffer midiPattern;
    // Here we track each time we get a noteOn event. This
    // way, at the end of the pattern we can fire noteOff
    // events for any monome buttons that are held longer
    // than the pattern
    MidiBuffer noteOffs;

    // Properties /////////////////////////////////////////////
    bool isPatternRecording, isPatternPlaying;
    bool isPatternStopping;
    bool doesPatternLoop;
    int patternLength, patternPrecountLength;
    int patternLengthInSamples, patternPrecountLengthInSamples;
    int patternPosition, patternPrecountPosition;
    int patternBank;

private:

    // Communication ///////////////////
    mlrVSTAudioProcessor * const parent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternRecording);
};



#endif  // __PATTERNRECORDING_H_E05E291D__
