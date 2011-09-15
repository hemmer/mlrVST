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
    numChunks(0), blockSize(0),
    currentChannel(0),
    isPlaying(false), playbackPercentage(0.0),
    currentPlayMode(LOOP), isReversed(false)
{

}

void SampleStrip::setCurrentSample(const AudioSample *newSample)
{
    if (newSample != 0)
    {
        currentSample = newSample;
        totalSampleLength = currentSample->getSampleLength();
        selectionStart = (int)(fractionalSampleStart * totalSampleLength);
        selectionEnd = (int)(fractionalSampleEnd * totalSampleLength);

        selectionLength = (selectionEnd - selectionStart);
        blockSize = (int) (selectionLength / (float) numChunks);
        //TODO do we set selection variables here?
    }
}

void SampleStrip::setSampleStripParam(const int &parameterID, const int &newValue)
{
    DBG("param \"" + getParameterName(parameterID) + "\" updated to: " + String(newValue));

    switch (parameterID)
    {
    case ParamCurrentChannel :
        currentChannel = newValue; break;
    case ParamNumChunks :
        numChunks = newValue;
        blockSize = (int) (int) (selectionLength / (float) numChunks); break;
    case ParamPlayMode :
        currentPlayMode = newValue; break;
    }
}

int SampleStrip::getSampleStripParam(const int &parameterID) const
{
    switch (parameterID)
    {
    case ParamCurrentChannel :
        return currentChannel; 
    case ParamNumChunks :
        return numChunks;
    case ParamPlayMode :
        return currentPlayMode;
    default: 
        return -1;      // this should never happen!
    }
}