/*
  ==============================================================================

    SampleStrip.cpp
    Created: 13 Sep 2011 1:07:11pm
    Author:  Hemmer

  ==============================================================================
*/

#include "SampleStrip.h"

SampleStrip::SampleStrip() :
    //stripID(newStripID),
    currentSample(0),
    sampleLength(0), sampleSelectionStart(0), sampleSelectionEnd(0),
    numBlocks(0), blockSize(0),
    currentChannel(0)
{

}

void SampleStrip::setCurrentSample(AudioSample *newSample)
{
    DBG("new sapjds");
    if (newSample != 0)
    {
        DBG("new sapjds LEGIT");
        currentSample = newSample;
        sampleLength = currentSample->getSampleLength();
        sampleStart = (int)(sampleSelectionStart * sampleLength);
        sampleEnd = (int)(sampleSelectionEnd * sampleLength);
        //TODO do we set selection variables here?
    }
}