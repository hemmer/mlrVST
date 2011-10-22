/*
  ==============================================================================

    SampleStrip.cpp
    Created: 13 Sep 2011 1:07:11pm
    Author:  Hemmer

  ==============================================================================
*/

#include <cmath>
#include "SampleStrip.h"

SampleStrip::SampleStrip() :
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
    case ParamCurrentChannel :
        currentChannel = *static_cast<const int*>(newValue); break;

    case ParamNumChunks :
        numChunks = *static_cast<const int*>(newValue);
        chunkSize = (int) (selectionLength / (float) numChunks); break;

    case ParamPlayMode :
        currentPlayMode = *static_cast<const int*>(newValue); break;
            
    case ParamIsLatched :
        isLatched = *static_cast<const bool*>(newValue); break;

    case ParamIsPlaying :
        isPlaying = *static_cast<const bool*>(newValue); break;

    case ParamIsReversed :
        isReversed = *static_cast<const bool*>(newValue); break;

    case ParamStripVolume :
        stripVolume = *static_cast<const float*>(newValue); break;

    case ParamPlaySpeed :
        playSpeed = *static_cast<const double*>(newValue); break;

    case ParamIsPlaySpeedLocked :
        isPlaySpeedLocked = *static_cast<const bool*>(newValue); break;

    case ParamPlaybackPercentage :
        playbackPercentage = *static_cast<const float*>(newValue); break;

    case ParamFractionalStart :
        fractionalSampleStart = *static_cast<const float*>(newValue);
        selectionStart = (int)(fractionalSampleStart * totalSampleLength);
        selectionLength = selectionEnd - selectionStart;
        chunkSize = (int) (selectionLength / (float) numChunks);
        break;

    case ParamFractionalEnd :
        fractionalSampleEnd = *static_cast<const float*>(newValue);
        selectionEnd = (int)(fractionalSampleEnd * totalSampleLength);
        selectionLength = selectionEnd - selectionStart;
        chunkSize = (int) (selectionLength / (float) numChunks);
        break;

    case ParamAudioSample :
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
        }
        else
        {
            playSpeed = 1.0;
            // SANITY CHECK: these values should never be used by processor!
            totalSampleLength = selectionStart = selectionEnd = selectionLength = chunkSize = 0;
            sampleSampleRate = 0.0;
        }
        break;

        // TODO when ParamSampleStart is changes, make sure we change fractional too!
    default :
        jassertfalse;     // we should NOT be here!
    }

    // notify listeners of changes if requested
    if (sendChangeMsg)
    {
        // DBG("param " << getParameterName(parameterID));
        sendChangeMessage();
    }
}

const void* SampleStrip::getSampleStripParam(const int &parameterID) const
{
    const void *p;

    switch (parameterID)
    {
    case ParamCurrentChannel :
        p = &currentChannel; break;

    case ParamNumChunks :
        p = &numChunks; break;

    case ParamPlayMode :
        p = &currentPlayMode; break;

    case ParamIsLatched :
        p = &isLatched; break;

    case ParamIsReversed :
        p = &isReversed; break;

    case ParamStripVolume :
        p = &stripVolume; break;

    case ParamPlaySpeed :
        p = &playSpeed; break;

    case ParamIsPlaySpeedLocked :
        p = &isPlaySpeedLocked; break;

    case ParamIsPlaying :
        p = &isPlaying; break;

    case ParamPlaybackPercentage :
        p = &playbackPercentage; break;

    case ParamFractionalStart :
        p = &fractionalSampleStart; break;

    case ParamFractionalEnd :
        p = &fractionalSampleEnd; break;

    // Audio processing only params
    case ParamChunkSize :
        p = &chunkSize; break;

    case ParamSampleStart :
        p = &selectionStart; break;

    case ParamSampleEnd :
        p = &selectionEnd; break;

    case ParamAudioSample :
        p = currentSample; break;

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
}

void SampleStrip::updatePlaySpeedForSelectionChange()
{

    if (!isPlaySpeedLocked && currentSample && selectionLength > 0)
    {
        jassert(previousSelectionLength > 0);
        playSpeed *= (selectionLength / (double) previousSelectionLength);
        previousSelectionLength = selectionLength;
    }
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
    }
}

void SampleStrip::modPlaySpeed(const double &factor)
{
    playSpeed *= factor;

    // let listeners know!
    sendChangeMessage();
}