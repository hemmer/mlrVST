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
                                   SampleStrip *initialSampleStrip,
                                   const int &totalSampleStrips) :
    parent(owner), numSampleStrips(totalSampleStrips),
    currentSampleStrip(initialSampleStrip),
    channelIDNumber(channelIDNo),
    channelGain(0.8f), stripGain(0.0f),
    currentSample(0), playSpeed(1.0), currentBPM(120.0f),
    sampleStartPosition(0), sampleEndPosition(0), selectionLength(0),
    sampleCurrentPosition(0.0),
    isPlaying(false),
    channelColour(col),
    isReversed(false), playMode(-1), chunkSize(-1), playbackStartPosition(-1),
    effectiveMonomeRow(-1), monomeCol(-1)
{

}

ChannelProcessor::~ChannelProcessor()
{
    parent.release();
}

void ChannelProcessor::setCurrentSampleStrip(SampleStrip* newSampleStrip)
{
    currentSampleStrip = newSampleStrip;
    //currentSample = currentSampleStrip->getCurrentSample();
    currentSample = static_cast<const AudioSample*>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamAudioSample));
}

void ChannelProcessor::startSamplePlaying(const int &startBlock, const int &blockSize)
{
    sampleStartPosition = startBlock * blockSize;
    sampleCurrentPosition = (double) sampleStartPosition;
    isPlaying = true;
}

void ChannelProcessor::stopSamplePlaying()
{
    bool stopPlay = false;
    currentSampleStrip->setSampleStripParam(SampleStrip::ParamIsPlaying, &stopPlay);
    isPlaying = false;
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
    if (m.getNoteNumber() == monomeCol && m.isNoteOff())
    {
        stopSamplePlaying();
    }

    /* NOTE: The -1 is here because the first monome row
       is reserved for mappings, making the second row
       effectively the first.
    */
    effectiveMonomeRow = m.getChannel() - 1;
    if (m.isNoteOn()) monomeCol = m.getNoteNumber();

    /* Load the new sample strip (this contains information
       about which sample to play etc). */
    setCurrentSampleStrip(parent->getSampleStrip(effectiveMonomeRow));
    
    playMode = *static_cast<const int*>
               (currentSampleStrip->getSampleStripParam(SampleStrip::ParamPlayMode));

    DBG("Current play mode: " << playMode);

    /* We are only interested in columns that are
       within the allowed number of chunks.
    */
    int numChunks = *static_cast<const int*>
                    (currentSampleStrip->getSampleStripParam(SampleStrip::ParamNumChunks));
    if (monomeCol < numChunks)
    {

        DBG("message received. note " + String(m.getNoteNumber()) + ", channel " + String(m.getChannel()));

        if (m.isNoteOn())
        {


            // We can save some effort by ignore cases where this is no sample!
            if (currentSample != 0)
            {

                // this gets the latest start and end points for the sample
                refreshPlaybackParameters();

                sampleCurrentPosition = (float) playbackStartPosition;
                isPlaying = true;

                float playbackPercentage = getCurrentPlaybackPercentage();
                currentSampleStrip->setSampleStripParam(SampleStrip::ParamIsPlaying, &isPlaying);
                currentSampleStrip->setSampleStripParam(SampleStrip::ParamPlaybackPercentage, &playbackPercentage);
            }

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


    if (currentSample != 0 && isPlaying)
    {
        // this gets the latest start and end points for the sample
        refreshPlaybackParameters();

        // TODO: can we remove a level of abstraction from here?
        const float* const inL = currentSample->getAudioData()->getSampleData(0, 0);
        const float* const inR = currentSample->getNumChannels() > 1
                               ? currentSample->getAudioData()->getSampleData(1, 0) : nullptr;

        float* outL = outputBuffer.getSampleData(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getSampleData (1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            // NEXT: what is sourceSamplePosition??????????????????
            const int pos = (int) sampleCurrentPosition;
            const double alpha = (double) (sampleCurrentPosition - pos);
            const double invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            // TODO should l/r be double
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

            sampleCurrentPosition += playSpeed;

            switch (playMode)
            {
            case SampleStrip::LOOP_CHUNK :
                if (sampleCurrentPosition > (playbackStartPosition + chunkSize))
                {
                    sampleCurrentPosition = (float) playbackStartPosition;
                    break;
                }
            case SampleStrip::PLAY_TO_END :
                if (sampleCurrentPosition > sampleEndPosition)
                {
                    stopSamplePlaying();
                    break;
                }
            case SampleStrip::LOOP :
                if (sampleCurrentPosition > sampleEndPosition)
                {
                    sampleCurrentPosition = (float) sampleStartPosition;
                    break;
                }
            }
        }

        float playbackPercentage = getCurrentPlaybackPercentage();
        currentSampleStrip->setSampleStripParam(SampleStrip::ParamPlaybackPercentage, &playbackPercentage);
    }
}

void ChannelProcessor::refreshPlaybackParameters()
{
    currentSample = static_cast<const AudioSample*>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamAudioSample));

    // Load sample strip details
    chunkSize = *static_cast<const int*>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamChunkSize));
    sampleStartPosition = *static_cast<const int*>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamSampleStart));
    sampleEndPosition = *static_cast<const int*>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamSampleEnd));
    // Then use the column to find which point to start at
    playbackStartPosition = sampleStartPosition + monomeCol * chunkSize;

    selectionLength = sampleEndPosition - sampleStartPosition;

    // If we reselect this keeps the currently playing point in sync
    if (sampleStartPosition > sampleCurrentPosition)
        sampleCurrentPosition = (double) playbackStartPosition;

    stripGain = *static_cast<const float *>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamStripVolume));

    isPlaying = *static_cast<const bool *>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamIsPlaying));

    playSpeed = *static_cast<const double *>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamPlaySpeed));

    playMode = *static_cast<const int *>
        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamPlayMode));
}