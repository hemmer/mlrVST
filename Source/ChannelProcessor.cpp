/*
  ==============================================================================

    ChannelProcessor.cpp
    Created: 8 Sep 2011 2:02:33pm
    Author:  Hemmer

  ==============================================================================
*/

#include "ChannelProcessor.h"


// main constructor
ChannelProcessor::ChannelProcessor(const int &channelIDNo, const Colour &col) :
    channelIDNumber(channelIDNo),
    channelGain(0.8f),
    currentSample(0),
    sampleStartPosition(0),
    isPlaying(false),
    isSampleReversed(false),
    channelColour(col)
{

}

void ChannelProcessor::setCurrentSample(const AudioSample* newSample)
{
    currentSample = newSample;
}

void ChannelProcessor::startSamplePlaying(const int &block, const int &blockSize)
{
    sampleStartPosition = block * blockSize;
    sampleCurrentPosition = sampleStartPosition;
    isPlaying = true;
}

void ChannelProcessor::stopSamplePlaying()
{
    isPlaying = false;
}


void ChannelProcessor::handleMidiEvent (const MidiMessage& m)
{
    /* PSUEDO CODE TO HELP DESIGN ONCE MONOME messages are interpreted

    // INCOMING PARAMS:

    use monomeRow to ignore messages coming for other channels:

    IF waveFormArray[monomeRow].getChannel != this.channelNumber
    THEN ignore msg
    ELSE setCurrentSample( waveFormArray[monomeRow].currentSample )

    then just monomeCol to choose sample start position based on PLAYBACk_MODE

    */



    if (m.isNoteOn())
    {
        int noteNumber = m.getNoteNumber();
        if(noteNumber >= 60 && noteNumber <= 67)
        {
            noteNumber -= 60;
            startSamplePlaying(noteNumber, 15000);
        }
    }
    else if (m.isNoteOff())
    {
        stopSamplePlaying();
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
        const bool useEvent = midiIterator.getNextEvent(m, midiEventPos)
                                && (midiEventPos < startSample + numSamples);

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
        if (currentSample != nullptr && isPlaying)
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

            if (sampleCurrentPosition > currentSample->getSampleLength())
            {
                stopSamplePlaying();
                break;
            }
        }
    }
}