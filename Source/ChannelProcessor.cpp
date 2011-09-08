/*
  ==============================================================================

    ChannelProcessor.cpp
    Created: 8 Sep 2011 2:02:33pm
    Author:  Hemmer

  ==============================================================================
*/

#include "ChannelProcessor.h"


// main constructor
ChannelProcessor::ChannelProcessor(const int &channelIDNo) :
    channelIDNumber(channelIDNo),
    currentSample(0),
    sampleStartPosition(0),
    isPlaying(false)
{

}

void ChannelProcessor::setCurrentSample(AudioSample &newSample)
{
    currentSample = &newSample;
}

void ChannelProcessor::startSamplePlaying(const int &block, const int &blockSize)
{
    sampleStartPosition = block * blockSize;
    isPlaying = true;
}

void ChannelProcessor::stopSamplePlaying()
{
    isPlaying = false;
}

// processes a block of samples [startSample, startSample + numSamples]
// TODO: eventually replace MIDIBuffer with MonomeDataBuffer
void ChannelProcessor::renderNextBlock(AudioSampleBuffer& outputBuffer,
                                   const MidiBuffer& midiData,
                                   int startSample,
                                   int numSamples)
{
   
    // must set the sample rate before using this!
    jassert (sampleRate != 0);

    const ScopedLock sl (lock);

    MidiBuffer::Iterator midiIterator(midiData);
    // only interested in MIDI data 
    midiIterator.setNextSamplePosition(startSample);
    MidiMessage m(0xf4, 0.0);

    while (numSamples > 0)
    {
        int midiEventPos;
        // try to find a corresponding MIDI event and see if it's within range
        const bool useEvent = midiIterator.getNextEvent(m, midiEventPos)
                                && (midiEventPos < startSample + numSamples);

        // if there was an event, process up until that position
        // otherwise process until the end
        const int numThisTime = useEvent ? midiEventPos - startSample
                                         : numSamples;

        if (numThisTime > 0)
        {
            // render the current section
            renderSection(outputBuffer, startSample, numThisTime);
        }

        if (useEvent)
            handleMidiEvent(m);

        startSample += numThisTime;
        numSamples -= numThisTime;
    }
}




void renderSection(AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
        if (currentSample != nullptr && isPlaying)
    {
        // TODO: can remove a level of abstraction from here
        const float* const inL = currentSample->getAudioData()->getSampleData(0, 0);
        const float* const inR = currentSample->getAudioData()->getNumChannels() > 1
                               ? currentSample->getAudioData()->getSampleData(1, 0) : nullptr;

        float* outL = outputBuffer.getSampleData(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getSampleData (1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            const int pos = (int) sourceSamplePosition;
            const float alpha = (float) (sourceSamplePosition - pos);
            const float invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = (inL [pos] * invAlpha + inL [pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR [pos] * invAlpha + inR [pos + 1] * alpha)
                                       : l;

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

            // if we have stereo samples
            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            sourceSamplePosition += pitchRatio;

            if (sourceSamplePosition > playingSound->length)
            {
                stopNote (false);
                break;
            }
        }
    }
}