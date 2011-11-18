/*
  ==============================================================================

    SampleStrip.cpp
    Created: 13 Sep 2011 1:07:11pm
    Author:  Hemmer

  ==============================================================================
*/

#include <cmath>
#include "SampleStrip.h"
#include "PluginProcessor.h"

SampleStrip::SampleStrip(const int &newID,
                         const int &newNumSampleStrips,
                         mlrVSTAudioProcessor *owner) :
    sampleStripID(newID), numSampleStrips(newNumSampleStrips),
    parent(owner),
    currentSample(0), sampleSampleRate(0.0),
    totalSampleLength(0), fractionalSampleStart(0), fractionalSampleEnd(0),
    numChunks(8), chunkSize(0),
    currentChannel(0),
    isPlaying(false), playbackPercentage(0.0),
    currentPlayMode(LOOP), isReversed(false), isLatched(true),
    stripVolume(1.0f), playSpeed(1.0), isPlaySpeedLocked(false),
    previousBPM(120.0), previousSelectionLength(0)
{

}

void SampleStrip::setSampleStripParam(const int &parameterID,
                                      const void *newValue,
                                      const bool &sendChangeMsg)
{

    switch (parameterID)
    {
    case pCurrentChannel :
        currentChannel = *static_cast<const int*>(newValue); break;

    case pNumChunks :
        numChunks = *static_cast<const int*>(newValue);
        chunkSize = (int) (selectionLength / (float) numChunks);
        updatePlayParams();
        break;

    case pPlayMode :
        currentPlayMode = *static_cast<const int*>(newValue); break;
            
    case pIsLatched :
        isLatched = *static_cast<const bool*>(newValue); break;

    case pIsPlaying :
        isPlaying = *static_cast<const bool*>(newValue); break;

    case pIsReversed :
        isReversed = *static_cast<const bool*>(newValue); break;

    case pStripVolume :
        stripVolume = *static_cast<const float*>(newValue); break;

    case pPlaySpeed :
        playSpeed = *static_cast<const double*>(newValue); break;

    case pIsPlaySpeedLocked :
        isPlaySpeedLocked = *static_cast<const bool*>(newValue); break;

    case pPlaybackPercentage :
        playbackPercentage = *static_cast<const float*>(newValue); break;

    case pFractionalStart :
        fractionalSampleStart = *static_cast<const float*>(newValue);
        selectionStart = (int)(fractionalSampleStart * totalSampleLength);
        selectionLength = selectionEnd - selectionStart;
        chunkSize = (int) (selectionLength / (float) numChunks);
        updatePlayParams();
        break;

    case pFractionalEnd :
        fractionalSampleEnd = *static_cast<const float*>(newValue);
        selectionEnd = (int)(fractionalSampleEnd * totalSampleLength);
        selectionLength = selectionEnd - selectionStart;
        chunkSize = (int) (selectionLength / (float) numChunks);
        updatePlayParams();
        break;

    case pStartChunk :
        loopStartChunk = *static_cast<const int*>(newValue);
        updatePlayParams();
        break;

    case pEndChunk :
        loopEndChunk = *static_cast<const int*>(newValue);
        updatePlayParams();
        break;

    case pAudioSample :
        currentSample = static_cast<const AudioSample*>(newValue);
        // update associated params if there is a sample
        if (currentSample)
        {
            totalSampleLength = currentSample->getSampleLength();
            selectionStart = (int)(fractionalSampleStart * totalSampleLength);
            selectionEnd = (int)(fractionalSampleEnd * totalSampleLength);
            selectionLength = (selectionEnd - selectionStart);
            chunkSize = (int) (selectionLength / (float) numChunks);
            sampleSampleRate = currentSample->getSampleRate();
            updatePlayParams();
        }
        else
        {
            playSpeed = 1.0;
            // SANITY CHECK: these values should never be used by processor!
            totalSampleLength = selectionStart = selectionEnd = selectionLength = chunkSize = 0;
            sampleSampleRate = 0.0;
        }
        break;

        // TODO when pSampleStart is changes, make sure we change fractional too!
    default :
        jassertfalse;     // we should NOT be here!
    }

    // notify listeners of changes if requested
    if (sendChangeMsg)
    {
        //DBG("param " << getParameterName(parameterID) << " sent by " << sampleStripID);
        sendChangeMessage();
    }
}

const void* SampleStrip::getSampleStripParam(const int &parameterID) const
{
    const void *p;

    switch (parameterID)
    {
    case pCurrentChannel :
        p = &currentChannel; break;

    case pNumChunks :
        p = &numChunks; break;

    case pPlayMode :
        p = &currentPlayMode; break;

    case pIsLatched :
        p = &isLatched; break;

    case pIsReversed :
        p = &isReversed; break;

    case pStripVolume :
        p = &stripVolume; break;

    case pPlaySpeed :
        p = &playSpeed; break;

    case pIsPlaySpeedLocked :
        p = &isPlaySpeedLocked; break;

    case pIsPlaying :
        p = &isPlaying; break;

    case pPlaybackPercentage :
        p = &playbackPercentage; break;

    case pFractionalStart :
        p = &fractionalSampleStart; break;

    case pFractionalEnd :
        p = &fractionalSampleEnd; break;

    // Audio processing only params
    case pChunkSize :
        p = &chunkSize; break;

    case pSampleStart :
        p = &selectionStart; break;

    case pSampleEnd :
        p = &selectionEnd; break;

    case pAudioSample :
        p = currentSample; break;

    case pStartChunk :
        p = &loopStartChunk; break;

    case pEndChunk :
        p = &loopEndChunk; break;

    default:
        DBG("Param not found!");
        jassertfalse;
        p = 0;      // this should never happen!
    }

    return p;
}

void SampleStrip::updatePlaySpeedForBPMChange(const double &newBPM)
{
    if (!isPlaySpeedLocked && currentSample)
    {
        playSpeed *= (newBPM / previousBPM);
        previousBPM = newBPM;
    }

    // let any listeners (i.e. GUI) know of the change
    sendChangeMessage();
}

void SampleStrip::updatePlaySpeedForSelectionChange()
{
    if (!isPlaySpeedLocked && currentSample && selectionLength > 0)
    {
        jassert(previousSelectionLength > 0);
        playSpeed *= (selectionLength / (double) previousSelectionLength);
        previousSelectionLength = selectionLength;
    }

    // let any listeners (i.e. GUI) know of the change
    sendChangeMessage();
}

void SampleStrip::findInitialPlaySpeed(const double &newBPM, const float &hostSampleRate)
{
    if (currentSample)
    {
        double numSamplesInFourBars = ((newBPM / 960.0) * hostSampleRate);
        playSpeed = (double)(selectionLength / sampleSampleRate) * (numSamplesInFourBars / hostSampleRate);

        /* This uses multiples of two to find the closest speed to 1.0,
           and sets the tempo to use in updatePlaySpeedForBPMChange()
           and updatePlaySpeedForSelectionChange()
        */
        previousBPM = newBPM;
        previousSelectionLength = selectionLength;

        double newPlaySpeed = playSpeed;
        if (playSpeed > 1.0)
        {
            while ( abs(newPlaySpeed / 2 - 1.0) < abs(newPlaySpeed - 1.0) )
            {
                newPlaySpeed /= 2.0;
            }
        }
        else
        {
            while ( abs(newPlaySpeed * 2 - 1.0) < abs(newPlaySpeed - 1.0) )
            {
                newPlaySpeed *= 2.0;
            }
        }
        playSpeed = newPlaySpeed;

        // let any listeners (i.e. GUI) know of the change
        sendChangeMessage();

    }
}

void SampleStrip::modPlaySpeed(const double &factor)
{
    playSpeed *= factor;

    // let listeners know!
    sendChangeMessage();
}





void SampleStrip::handleMidiEvent (const MidiMessage& m)
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
    if (m.isNoteOff())
    {
        if (!isLatched)
            stopSamplePlaying();
    }
    else if (currentSample && m.isNoteOn())
    {
        initialColumn = m.getNoteNumber();

        if (initialColumn < numChunks)
        {
            startSamplePlaying(initialColumn);
        }
    }
}


void SampleStrip::renderNextBlock(AudioSampleBuffer& outputBuffer,
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
            // filter out dodgy input
            if (stripRow < 0 || stripRow > numSampleStrips) break;

            if ((stripRow == sampleStripID) &&
                (midiEventPos < startSample + numSamples))
            {
                useEvent = true;
            }

            // Then get the channel associated with that sample strip
            int messageSelChannel = *static_cast<const int*>
                (parent->getSampleStrip(stripRow)->getSampleStripParam(SampleStrip::pCurrentChannel));

            // If a different strip is using the same channel
            // then stop THIS strip from playing.
            if (isPlaying && (midiEventPos < startSample + numSamples) &&
                (stripRow != sampleStripID) && (messageSelChannel == currentChannel))
                stopSamplePlaying();
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
    updateCurrentPlaybackPercentage();
}


void SampleStrip::renderNextSection(AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{

    if (currentSample != 0 && isPlaying)
    {
        updatePlayParams();

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

            /* TODO: temporary hack to fix pop. This seems to happen when the pointer
               deferences something not part of the sample by mistake, leading to a value
               of order 1000000! This fix will do in lieu of a proper solution:
            */
            if (l < -1.0f || l > 1.0f) l = 0.0f; 
            if (r < -1.0f || r > 1.0f) r = 0.0f;
            
            l *=  stripVolume;
            r *=  stripVolume;

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
                //DBG("#2 " << currentPlayMode << " strip " << sampleStripID << " " << playSpeed);

                switch (currentPlayMode)
                {

                case SampleStrip::LOOP :
                    if (sampleCurrentPosition > playbackEndPosition)
                        sampleCurrentPosition = (float) playbackStartPosition;

                    break;

                case SampleStrip::PLAY_TO_END :
                    if (sampleCurrentPosition > playbackEndPosition)
                        stopSamplePlaying();

                    break;

                case SampleStrip::PLAY_CHUNK_ONCE :
                    if (sampleCurrentPosition > selectionStart + (initialColumn + 1) * chunkSize)
                        stopSamplePlaying();

                    break;

                default : jassertfalse;
                }
            }
            else
            {
                // go back in time...
                sampleCurrentPosition -= playSpeed;
                //DBG("#2 " << sampleCurrentPosition << " strip " << sampleStripID << " " << playSpeed);

                switch (currentPlayMode)
                {

                case SampleStrip::LOOP :
                    if (sampleCurrentPosition < playbackEndPosition)
                        sampleCurrentPosition = (float) playbackStartPosition;
                    break;

                case SampleStrip::PLAY_TO_END :
                    if (sampleCurrentPosition < playbackEndPosition)
                        stopSamplePlaying();

                    break;

                case SampleStrip::PLAY_CHUNK_ONCE :
                    if (sampleCurrentPosition < selectionStart + (initialColumn - 1) * chunkSize)
                        stopSamplePlaying();
                    break;

                default : jassertfalse;

                }
            }
        }
    }
}


void SampleStrip::startSamplePlaying(const int &chunk)
{
    
    bool newPlayStatus = true;
    // this is to much sure listeners are informed
    setSampleStripParam(pIsPlaying, &newPlayStatus);

    if (!isReversed)
    {
        playbackStartPosition = selectionStart + loopStartChunk * chunkSize;
        playbackEndPosition = selectionStart + loopEndChunk * chunkSize;
        sampleCurrentPosition = (float) selectionStart + chunk * chunkSize;
    }
    else 
    {
        playbackStartPosition = selectionStart + loopEndChunk * chunkSize;
        playbackEndPosition = selectionStart + loopStartChunk * chunkSize;
        sampleCurrentPosition = (float) selectionStart + (chunk + 1) * chunkSize;
    }

}

double SampleStrip::stopSamplePlaying()
{
    bool newPlayStatus = false;
    setSampleStripParam(pIsPlaying, &newPlayStatus);
    return sampleCurrentPosition;
}

void SampleStrip::updatePlayParams()
{

    if (!isReversed)
    {
        playbackStartPosition = selectionStart + loopStartChunk * chunkSize;
        playbackEndPosition = selectionStart + loopEndChunk * chunkSize;

        if (sampleCurrentPosition > playbackEndPosition)
            sampleCurrentPosition = (double) playbackStartPosition;
        else if (sampleCurrentPosition < playbackStartPosition)
            sampleCurrentPosition = (double) playbackStartPosition;

    }
    else 
    {
        playbackStartPosition = selectionStart + loopEndChunk * chunkSize;
        playbackEndPosition = selectionStart + loopStartChunk * chunkSize;

        if (sampleCurrentPosition < playbackEndPosition)
            sampleCurrentPosition = (double) playbackStartPosition;
        else if (sampleCurrentPosition > playbackStartPosition)
            sampleCurrentPosition = (double) playbackStartPosition;

    }

}

void SampleStrip::updateCurrentPlaybackPercentage()
{
    float newPlaybackPercentage;

    if (isPlaying && (selectionStart != selectionEnd))
    {
        newPlaybackPercentage = (float) (sampleCurrentPosition - selectionStart)
            / (float) (selectionEnd - selectionStart);
    }
    else
        newPlaybackPercentage = -1.0;

    setSampleStripParam(pPlaybackPercentage, &newPlaybackPercentage, false);
}