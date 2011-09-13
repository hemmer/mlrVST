/*
  ==============================================================================

    SampleStrip.h
    Created: 13 Sep 2011 1:07:11pm
    Author:  Hemmer

    This class is the logic version equivalent of WaveformControl.

  ==============================================================================
*/

#ifndef __SAMPLESTRIP_H_ACB589A6__
#define __SAMPLESTRIP_H_ACB589A6__

#include "AudioSample.h"

class SampleStrip
{
public:
    SampleStrip();
    ~SampleStrip()
    {
        // make sure we don't unload the sample twice!
        currentSample.release();
    }

    enum SampleStripParam
    {

    };

    AudioSample* getCurrentSample() { return currentSample; }
    void setCurrentSample(AudioSample *newSample);

    // getters / setters
    int getSampleLength() const
    {
        return sampleLength;
    }
    int getSampleStart() const
    {
        return sampleStart;
    }
    int getSampleEnd() const
    {
        return sampleEnd;
    }
    int getNumBlocks() const
    {
        return numBlocks;
    }
    int getBlockSize() const
    {
        return blockSize;
    }
    int getCurrentChannel() const
    {
        return currentChannel;
    }
    //int getStripID() const { return stripID; }

    // void setSampleLength(const int &newSampleLength) { sampleLength = newSampleLength; }

    void setSampleSelection(const float &start, const float &end)
    {
        sampleSelectionStart = start;
        sampleSelectionEnd = end;

        // TODO: check this rounding is OK
        sampleStart = (int)(sampleSelectionStart * sampleLength);
        sampleEnd = (int)(sampleSelectionEnd * sampleLength);

        DBG("sample start: " + String(sampleStart) + " " + String(sampleSelectionStart));
        DBG("sample end: " + String(sampleEnd) + " " + String(sampleSelectionEnd));
    }

    void setNumBlocks(const int &newNumBlocks)
    {
        numBlocks = newNumBlocks;
        blockSize = (int)(sampleLength / (double) numBlocks);
    }
    void setCurrentChannel(const int &newChannel)
    {
        currentChannel = newChannel;
    }

private:

    //int stripID;

    // Pointer to currently selected sample
    ScopedPointer<AudioSample> currentSample;

    int sampleLength;
    // start / end points (fractional)
    float sampleSelectionStart, sampleSelectionEnd;
    // start / end points (in samples)
    int sampleStart, sampleEnd;
    int numBlocks, blockSize;

    int currentChannel;
};



#endif  // __SAMPLESTRIP_H_ACB589A6__
