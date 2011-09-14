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

    // TODO: typedef make this short to type
    enum SampleStripParameter
    {
        ParamCurrentChannel,
        ParamNumChunks
    };

    String getParameterName(const int &parameterID) const
    {
        switch(parameterID)
        {
        case ParamCurrentChannel : return "current channel";
        case ParamNumChunks : return "num chunks";
        default : return "parameter not found";
        }
    }


    AudioSample* getCurrentSample() const { return currentSample; }
    void setCurrentSample(AudioSample *newSample);

    //int getSampleLength() const { return sampleLength; }

    // these return the key sample points (taking selection into account)
    int getSampleStart() const { return selectionStart; }
    int getSampleEnd() const { return selectionEnd; }
    int getBlockSize() const { return blockSize; }
    int getCurrentChannel() const { return currentChannel; }

    // TODO: have this set through setParameter
    int getNumChunks() const { return numChunks; }

    void setSampleSelection(const float &start, const float &end)
    {
        fractionalSampleStart = start;
        fractionalSampleEnd = end;

        // TODO: check this rounding is OK
        selectionStart = (int)(fractionalSampleStart * totalSampleLength);
        selectionEnd = (int)(fractionalSampleEnd * totalSampleLength);
        selectionLength = selectionEnd - selectionStart;
        blockSize = (int) (selectionLength / (float) numChunks);

        DBG("sample start: " + String(selectionStart) + " " + String(fractionalSampleStart));
        DBG("sample end: " + String(selectionEnd) + " " + String(fractionalSampleEnd));
    }

    void setSampleStripParameter(const int &parameterID, const int &newValue);

private:

    // Pointer to currently selected sample
    ScopedPointer<AudioSample> currentSample;
    int totalSampleLength;

    // start / end points (fractional, i.e. 0.5 is half way through)
    float fractionalSampleStart, fractionalSampleEnd;
    // start / end / length of the selection (in samples)
    int selectionStart, selectionEnd, selectionLength;

    int numChunks, blockSize;

    int currentChannel;
};



#endif  // __SAMPLESTRIP_H_ACB589A6__
