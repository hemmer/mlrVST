/*
  ==============================================================================

    ChannelProcessor.cpp
    Created: 8 Sep 2011 2:02:33pm
    Author:  Hemmer

  ==============================================================================
*/

#include "ChannelProcessor.h"
#include "PluginProcessor.h"


// main constructor
ChannelProcessor::ChannelProcessor(const int &channelIDNo,
                                   const Colour &col,
                                   mlrVSTAudioProcessor *owner,
                                   const int &totalSampleStrips) :
    parent(owner), numSampleStrips(totalSampleStrips),
    currentSampleStrip(0),
    channelIDNumber(channelIDNo),
    channelGain(0.8f), stripGain(0.0f),
    currentSample(0), playSpeed(1.0), currentBPM(120.0f),
    sampleStartPosition(0), sampleEndPosition(0),
    sampleCurrentPosition(0.0),
    isPlaying(false),
    channelColour(col),
    isReversed(false), playMode(-1), chunkSize(-1), playbackStartPosition(-1),
    effectiveMonomeRow(-1), monomeCol(-5)
{

}

ChannelProcessor::~ChannelProcessor()
{
    parent.release();
}

void ChannelProcessor::setCurrentSampleStrip(SampleStrip* newSampleStrip)
{
    currentSampleStrip = newSampleStrip;
    currentSample = static_cast<const AudioSample*>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamAudioSample));

}

/* Switching Channels during playback doens't work for now but it will eventually use this */
//void ChannelProcessor::startSamplePlaying(const int &newStartPosition, SampleStrip * const newSampleStrip)
//{
//    currentSampleStrip = newSampleStrip;
//    // this gets the latest start and end points for the sample
//    refreshPlaybackParameters();
//
//    sampleCurrentPosition = (float) newStartPosition;
//    isPlaying = true;
//
//    float playbackPercentage = getCurrentPlaybackPercentage();
//    currentSampleStrip->setSampleStripParam(SampleStrip::ParamIsPlaying, &isPlaying);
//    currentSampleStrip->setSampleStripParam(SampleStrip::ParamPlaybackPercentage, &playbackPercentage);
//}

// If no argument given start at playbackStartPosition
void ChannelProcessor::startSamplePlaying()
{
    // this gets the latest start and end points for the sample
    refreshPlaybackParameters();

    sampleCurrentPosition = (float) playbackStartPosition;
    isPlaying = true;

    float playbackPercentage = getCurrentPlaybackPercentage();
    currentSampleStrip->setSampleStripParam(SampleStrip::ParamIsPlaying, &isPlaying);
    currentSampleStrip->setSampleStripParam(SampleStrip::ParamPlaybackPercentage, &playbackPercentage);
}


// Returns the point at which the sample stopped
double ChannelProcessor::stopSamplePlaying()
{
    bool stopPlay = false;

    // It's possible that other sources have stopped the sample already
    if (currentSampleStrip)
        currentSampleStrip->setSampleStripParam(SampleStrip::ParamIsPlaying, &stopPlay);

    currentSampleStrip = 0;
    currentSample = 0;
    isPlaying = false;
    return sampleCurrentPosition;
}


void ChannelProcessor::handleMidiEvent (const MidiMessage& m)
{
    /* Due to the design of JUCE, for now OSC messages are handled
       as MIDI messages. The spec is:

       monomeRow - MIDI message channel
       monomeCol - MIDI message note (0-15)                 */
    

    /* First stop any existing samples from playing. It
       doesn't actually matter if it is a note on or off
       message as we would stop the current sample for 
       either a button lift or the arrive of a new sample
       from a different strip.              
    */
    if (m.getNoteNumber() == monomeCol && m.isNoteOff() && currentSampleStrip)
    {
        bool isStripLatched = *static_cast<const bool*>
            (currentSampleStrip->getSampleStripParam(SampleStrip::ParamIsLatched));
        
        if (!isStripLatched)
            stopSamplePlaying();
    }

    if (m.isNoteOn())
    {
        /* NOTE: The -1 is here because the first monome row
        is reserved for mappings, making the second row
        effectively the first.
        */
        effectiveMonomeRow = m.getChannel() - 1;
        monomeCol = m.getNoteNumber();

        /* Load the new sample strip (this contains information
        about which sample to play etc). */
        setCurrentSampleStrip(parent->getSampleStrip(effectiveMonomeRow));

        playMode = *static_cast<const int*>
            (currentSampleStrip->getSampleStripParam(SampleStrip::ParamPlayMode));

        /* We are only interested in columns that are
        within the allowed number of chunks.
        */
        int numChunks = *static_cast<const int*>
            (currentSampleStrip->getSampleStripParam(SampleStrip::ParamNumChunks));

        if (monomeCol < numChunks)
        {
            // We can save some effort by ignore cases where this is no sample!
            if (currentSample != 0)
                startSamplePlaying();
        }
    }
}


// processes a block of samples [startSample, startSample + numSamples]
// TODO: eventually replace MIDIBuffer with MonomeDataBuffer
void ChannelProcessor::renderNextBlock(AudioSampleBuffer& outputBuffer,
                                   const MidiBuffer& midiData,
                                   int startSample,
                                   int numSamples)
{
   
    // must set the sample rate before using this!
    //jassert (sampleRate != 0);
    
    const ScopedLock sl (lock);

    MidiBuffer::Iterator midiIterator(midiData);
    // only interested in MIDI data 
    midiIterator.setNextSamplePosition(startSample);
    MidiMessage m(0xf4, 0.0);

    while (numSamples > 0)
    {
        int midiEventPos;
        // try to find a corresponding MIDI event and see if it's within range
        // and is the correct channel
        bool useEvent = false;
        if(midiIterator.getNextEvent(m, midiEventPos))
        {
            // First get the sample strip from the channel
            int stripRow = m.getChannel() - 1;
            if (stripRow < 0 || stripRow > numSampleStrips) break;

            // Then get the channel associated with that sample strip
            int messageSelChannel = *static_cast<const int*>
                (parent->getSampleStrip(stripRow)->getSampleStripParam(SampleStrip::ParamCurrentChannel));

            // If the channel matches THIS channel then use the event
            if ((midiEventPos < startSample + numSamples) &&
                (messageSelChannel == channelIDNumber))
                useEvent = true;
        }

        // if there was an event, process up until that position
        // otherwise process until the end
        const int numThisTime = useEvent ? midiEventPos - startSample
                                         : numSamples;

        if (numThisTime > 0)
        {
            // render the current section
            renderNextSection(outputBuffer, startSample, numThisTime);
        }

        if (useEvent)
            handleMidiEvent(m);

        startSample += numThisTime;
        numSamples -= numThisTime;
    }
}




void ChannelProcessor::renderNextSection(AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    // see if any parameters have changed    
    refreshPlaybackParameters();

    if (currentSample != 0 && isPlaying)
    {
        const float* const inL = currentSample->getAudioData()->getSampleData(0, 0);
        const float* const inR = currentSample->getNumChannels() > 1
            ? currentSample->getAudioData()->getSampleData(1, 0) : nullptr;

        float* outL = outputBuffer.getSampleData(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getSampleData (1, startSample) : nullptr;

        while (--numSamples >= 0)
        {

            // just using a very simple linear interpolation here..
            const int pos = (int) sampleCurrentPosition;
            const double alpha = (double) (sampleCurrentPosition - pos);
            const double invAlpha = 1.0f - alpha;


            // double up if mono
            float l = (float)(inL [pos] * invAlpha + inL [pos + 1] * alpha);
            float r = (inR != nullptr) ? (float)(inR [pos] * invAlpha + inR [pos + 1] * alpha) : l;

            l *=  channelGain * stripGain;
            r *=  channelGain * stripGain;

            // if we have stereo output avaiable 
            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                // else average the two channels
                *outL++ += (l + r) * 0.5f;
            }

            

            if (!isReversed)
            {
                sampleCurrentPosition += playSpeed;

                switch (playMode)
                {
                case SampleStrip::LOOP_CHUNK :
                    if (sampleCurrentPosition > (playbackStartPosition + chunkSize))
                    {
                        sampleCurrentPosition = (float) playbackStartPosition; break;
                    }
                case SampleStrip::PLAY_TO_END :
                    if (sampleCurrentPosition > sampleEndPosition)
                    {
                        stopSamplePlaying(); break;
                    }
                case SampleStrip::LOOP :
                    if (sampleCurrentPosition > sampleEndPosition)
                    {
                        sampleCurrentPosition = (float) sampleStartPosition; break;
                    }
                }
            }
            else
            {
                // go back in time...
                sampleCurrentPosition -= playSpeed;

                if (sampleCurrentPosition < playbackStartPosition)
                {
                    switch (playMode)
                    {
                    case SampleStrip::LOOP_CHUNK :
                        sampleCurrentPosition = (float)(playbackStartPosition + chunkSize); break;

                    case SampleStrip::PLAY_TO_END :
                        stopSamplePlaying(); break;

                    case SampleStrip::LOOP :
                        sampleCurrentPosition = (float) sampleEndPosition; break;
                    }
                }
            }
        }
    }
}


void ChannelProcessor::refreshPlaybackParameters()
{
    if (currentSampleStrip)
    {
        currentSample = static_cast<const AudioSample*>
            (currentSampleStrip->getSampleStripParam(SampleStrip::ParamAudioSample));

        float playbackPercentage = getCurrentPlaybackPercentage();
        currentSampleStrip->setSampleStripParam(SampleStrip::ParamPlaybackPercentage, &playbackPercentage);


        if (currentSample)
        {
            // Load sample strip details
            chunkSize = *static_cast<const int*>
                (currentSampleStrip->getSampleStripParam(SampleStrip::ParamChunkSize));
            sampleStartPosition = *static_cast<const int*>
                (currentSampleStrip->getSampleStripParam(SampleStrip::ParamSampleStart));
            sampleEndPosition = *static_cast<const int*>
                (currentSampleStrip->getSampleStripParam(SampleStrip::ParamSampleEnd));
            // Then use the column to find which point to start at
            playbackStartPosition = sampleStartPosition + monomeCol * chunkSize;

            // if chunksize increases this can happen
            if (playbackStartPosition > sampleEndPosition)
                playbackStartPosition = sampleStartPosition;

            // If we reselect this keeps the currently playing point in sync
            // Also if the new sample is shorted
            if ((sampleStartPosition > sampleCurrentPosition) || (sampleCurrentPosition > sampleEndPosition))
                sampleCurrentPosition = (double) playbackStartPosition;


            stripGain = *static_cast<const float *>
                (currentSampleStrip->getSampleStripParam(SampleStrip::ParamStripVolume));

            isPlaying = *static_cast<const bool *>
                (currentSampleStrip->getSampleStripParam(SampleStrip::ParamIsPlaying));

            isReversed = *static_cast<const bool *>
                (currentSampleStrip->getSampleStripParam(SampleStrip::ParamIsReversed));

            playSpeed = *static_cast<const double *>
                (currentSampleStrip->getSampleStripParam(SampleStrip::ParamPlaySpeed));

            playMode = *static_cast<const int *>
                (currentSampleStrip->getSampleStripParam(SampleStrip::ParamPlayMode));
        }
    }
}