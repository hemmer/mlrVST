/*
  ==============================================================================

    SampleStrip.cpp
    Created: 13 Sep 2011 1:07:11pm
    Author:  Hemmer

  ==============================================================================
*/

#include "SampleStrip.h"

SampleStrip::SampleStrip() :
    currentSample(0),
    totalSampleLength(0), fractionalSampleStart(0), fractionalSampleEnd(0),
    numChunks(8), chunkSize(0),
    currentChannel(0),
    isPlaying(false), playbackPercentage(0.0),
    currentPlayMode(LOOP_CHUNK), isReversed(false)
{

}

void SampleStrip::setSampleStripParam(const int &parameterID, const void *newValue)
{
    //DBG("param \"" + getParameterName(parameterID) + "\" updated to: " + String(newValue));

    switch (parameterID)
    {
    case ParamCurrentChannel :
        currentChannel = *static_cast<const int*>(newValue);
        DBG("SampleStrip set to chan: " << currentChannel);
        break;

    case ParamNumChunks :
        numChunks = *static_cast<const int*>(newValue);
        chunkSize = (int) (selectionLength / (float) numChunks); break;

    case ParamPlayMode :
        currentPlayMode = *static_cast<const int*>(newValue); break;

    case ParamIsPlaying :
        isPlaying = *static_cast<const bool*>(newValue); break;

    case ParamIsReversed :
        isReversed = *static_cast<const bool*>(newValue); break;

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
        // update associated params
        totalSampleLength = currentSample->getSampleLength();
        selectionStart = (int)(fractionalSampleStart * totalSampleLength);
        selectionEnd = (int)(fractionalSampleEnd * totalSampleLength);
        selectionLength = (selectionEnd - selectionStart);
        chunkSize = (int) (selectionLength / (float) numChunks);
        break;

        // TODO when ParamSampleStart is changes, make sure we change fractional too!
    default :
        jassert(false);     // we should NOT be here!
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
    case ParamIsReversed :
        p = &isReversed; break;
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
        jassert(false);
        p = 0;      // this should never happen!
    }

    return p;
}