/*
  ==============================================================================

    ChannelProcessor.h
    Created: 8 Sep 2011 2:02:33pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __CHANNELPROCESSOR_H_C271EC2B__
#define __CHANNELPROCESSOR_H_C271EC2B__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"
#include "AudioSample.h"

class ChannelProcessor

{
public:

    // Default constructor
    ChannelProcessor();

    // Normal constructor
    ChannelProcessor(const int &channelIDNo);
                     
    //==============================================================================
    /** Renders the next section of data for this voice.

        The output audio data must be added to the current contents of the buffer provided.
        Only the region of the buffer between startSample and (startSample + numSamples)
        should be altered by this method.

        If the voice is currently silent, it should just return without doing anything.

        If the sound that the voice is playing finishes during the course of this rendered
        block, it must call clearCurrentNote(), to tell the synthesiser that it has finished.

        The size of the blocks that are rendered can change each time it is called, and may
        involve rendering as little as 1 sample at a time. In between rendering callbacks,
        the voice's methods will be called to tell it about note and controller events.
    */
    void renderNextSection(AudioSampleBuffer& outputBuffer, int startSample, int numSamples);


    /** Creates the next block of audio output.

        This will process the next numSamples of data from all the voices, and add that output
        to the audio block supplied, starting from the offset specified. Note that the
        data will be added to the current contents of the buffer, so you should clear it
        before calling this method if necessary.

        The midi events in the inputMidi buffer are parsed for note and controller events,
        and these are used to trigger the voices. Note that the startSample offset applies
        both to the audio output buffer and the midi input buffer, so any midi events
        with timestamps outside the specified region will be ignored.
    */
    void renderNextBlock(AudioSampleBuffer& outputBuffer,
                         const MidiBuffer& midiData,
                         int startSample,
                         int numSamples);



    //======================================================
    /** Plays a from a specific block point until the end of
        the sample.

        By default the sample is divided into 8 blocks

        block is which segment is being played from
        blockSize is the size of each block in samples
    */
    void startSamplePlaying(const int &block, const int &blockSize);

    void stopSamplePlaying();

    // The lets the ChannelProcessor know which sample to read
    void setCurrentSample(const AudioSample &currentSample);

private:

    // each channel has an individual ID
    const int channelIDNumber;

    // pointer to the current sample in the sample pool
    // NOTE: "const" qualifier means we can't change the
    // sample which this pointer points at
    const AudioSample *currentSample;

    // which position in the sample to start playing from (measured in samples)
    int sampleStartPosition;
    // where we are in the sample at the moment
    int sampleCurrentPosition;

    // status of sample playback
    bool isInAttack, isInRelease, isPlaying;

    void handleMidiEvent(const MidiMessage& m);


    //==============================================================================
    /** This is used to control access to the rendering callback and the note trigger methods. */
    CriticalSection lock;
};





#endif  // __CHANNELPROCESSOR_H_C271EC2B__
