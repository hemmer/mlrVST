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
                                   SampleStrip *initialSampleStrip) :
    parent(owner),
    channelIDNumber(channelIDNo),
    channelGain(0.8f),
    currentSample(0),
    sampleStartPosition(0), sampleEndPosition(0), selectionLength(0),
    isPlaying(false),
    channelColour(col),
    currentSampleStrip(initialSampleStrip),
    isReversed(false), playMode(-1), chunkSize(-1), playbackStartPosition(-1)
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
    sampleCurrentPosition = sampleStartPosition;
    isPlaying = true;
}

void ChannelProcessor::stopSamplePlaying()
{
    currentSampleStrip->setPlaybackStatus(false);
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
    stopSamplePlaying();

    /* NOTE: The -1 is here because the first monome
       row is reserved for mappings, making the second
       row effectively the first.
    */
    int effectiveMonomeRow = m.getChannel() - 1;
    int monomeCol = m.getNoteNumber();

    /* Load the new sample strip (this contains information
       about which sample to play etc). */
    setCurrentSampleStrip(parent->getSampleStrip(effectiveMonomeRow));
    
    playMode = *static_cast<const int*>
               (currentSampleStrip->getSampleStripParam(SampleStrip::ParamPlayMode));

    DBG("Current play mode: " + String());

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

            // Load sample strip details
            chunkSize = *static_cast<const int*>
                        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamChunkSize));
            sampleStartPosition = *static_cast<const int*>
                        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamSampleStart));
            sampleEndPosition = *static_cast<const int*>
                        (currentSampleStrip->getSampleStripParam(SampleStrip::ParamSampleEnd));

            // Then use the column to find which point to start at
            playbackStartPosition = sampleStartPosition + monomeCol * chunkSize;

            // We can save some effort by ignore cases where this is no sample!
            if (currentSample != 0)
            {
                sampleCurrentPosition = playbackStartPosition;
                isPlaying = true;
                currentSampleStrip->setPlaybackStatus(true);
                currentSampleStrip->setPlaybackPercentage(getCurrentPlaybackPercentage());
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
            // Get the selected channel associated with the row
            // as determined from MIDI message m
            int messageSelChannel = *static_cast<const int*>
                (parent->getSampleStrip(m.getChannel() - 1)->getSampleStripParam(SampleStrip::ParamCurrentChannel));

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
    if (currentSample == 0)
    {
        //DBG("null: " + String(channelIDNumber) + " " + String(isPlaying));
    } else
    {
        // DBG("good: " + String(channelIDNumber));
    }

    if (currentSample != 0 && isPlaying)
    {
        
        // TODO: can we remove a level of abstraction from here?
        const float* const inL = currentSample->getAudioData()->getSampleData(0, 0);
        const float* const inR = currentSample->getNumChannels() > 1
                               ? currentSample->getAudioData()->getSampleData(1, 0) : nullptr;

        float* outL = outputBuffer.getSampleData(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getSampleData (1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            // NEXT: what is sourceSamplePosition??????????????????
            //const int pos = (int) sourceSamplePosition;
            //const float alpha = (float) (sourceSamplePosition - pos);
            //const float invAlpha = 1.0f - alpha;
            // just using a very simple linear interpolation here..
            //float l = (inL [pos] * invAlpha + inL [pos + 1] * alpha);
            //float r = (inR != nullptr) ? (inR [pos] * invAlpha + inR [pos + 1] * alpha)
            //                           : l;

            // we only have one velocity
            //l *= lgain;
            //r *= rgain;
            //
            //if (isInAttack)
            //{
            //    l *= attackReleaseLevel;
            //    r *= attackReleaseLevel;

            //    attackReleaseLevel += attackDelta;

            //    if (attackReleaseLevel >= 1.0f)
            //    {
            //        attackReleaseLevel = 1.0f;
            //        isInAttack = false;
            //    }
            //}
            //else if (isInRelease)
            //{
            //    l *= attackReleaseLevel;
            //    r *= attackReleaseLevel;

            //    attackReleaseLevel += releaseDelta;

            //    if (attackReleaseLevel <= 0.0f)
            //    {
            //        stopNote (false);
            //        break;
            //    }
            //}


            float l = inL[sampleCurrentPosition];
            // if no right channel, clone in the left channel
            float r = (inR != nullptr) ? inR[sampleCurrentPosition] : l;

            l *=  channelGain;
            r *=  channelGain;

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

            sampleCurrentPosition += 1;

            switch (playMode)
            {
            case SampleStrip::LOOP_CHUNK :
                if (sampleCurrentPosition > (playbackStartPosition + chunkSize))
                {
                    sampleCurrentPosition = playbackStartPosition;
                    break;
                }
            case SampleStrip::LOOP :
                if (sampleCurrentPosition > sampleEndPosition)
                {
                    stopSamplePlaying();
                    break;
                }
            case SampleStrip::LOOP_WHILE_HELD :
                if (sampleCurrentPosition > sampleEndPosition)
                {
                    sampleCurrentPosition = sampleStartPosition;
                    break;
                }
            }
        }
        currentSampleStrip->setPlaybackPercentage(getCurrentPlaybackPercentage());
    }
}