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
    }

    // TODO: typedef make this short to type
    enum SampleStripParameter
    {
        ParamCurrentChannel,
        ParamNumChunks,
        ParamPlayMode
    };

    enum PlayMode
    { 
        LOOP = 1,                   // starts the sample looping
        LOOP_WHILE_HELD,        // same but only while button held
        LOOP_CHUNK,             // loops the chunk associated with a button
        PLAY_TO_END             // plays from current point until the end then stops
    };


    String getParameterName(const int &parameterID) const
    {
        switch(parameterID)
        {
        case ParamCurrentChannel : return "current channel";
        case ParamNumChunks : return "num chunks";
        case ParamPlayMode : return "playmode";
        default : return "parameter not found";
        }
    }


    const AudioSample* getCurrentSample() const { return currentSample; }
    void setCurrentSample(const AudioSample *newSample);

    //int getSampleLength() const { return sampleLength; }

    // these return the key sample points (taking selection into account)
    int getSampleStart() const { return selectionStart; }
    int getSampleEnd() const { return selectionEnd; }
    int getBlockSize() const { return blockSize; }
    int getCurrentChannel() const { return currentChannel; }

    void setSampleSelection(const float &start, const float &end)
    {
        fractionalSampleStart = start;
        fractionalSampleEnd = end;

        // TODO: check this rounding is OK
        selectionStart = (int)(fractionalSampleStart * totalSampleLength);
        selectionEnd = (int)(fractionalSampleEnd * totalSampleLength);
        selectionLength = selectionEnd - selectionStart;
        blockSize = (int) (selectionLength / (float) numChunks);
    }

    // these are for getting / setting the int parameters
    void setSampleStripParam(const int &parameterID, const int &newValue);
    int getSampleStripParam(const int &parameterID) const;

    void setPlaybackPercentage(const float &newPlaybackPercentage) { playbackPercentage = newPlaybackPercentage; }
    void setPlaybackStatus(const bool &isPlaying_) { isPlaying = isPlaying_; }

    bool getPlaybackStatus() const { return isPlaying; }
    float getPlaybackPercentage() const { return playbackPercentage; }

private:

    // These are just so the GUI can show where in the sample we are
    bool isPlaying;
    float playbackPercentage;

    // Pointer to currently selected sample
    const AudioSample *currentSample;
    int totalSampleLength;

    // start / end points (fractional, i.e. 0.5 is half way through)
    float fractionalSampleStart, fractionalSampleEnd;
    // start / end / length of the selection (in samples)
    int selectionStart, selectionEnd, selectionLength;

    // How many blocks the sample is split up into...
    int numChunks;
    // ...and what size are they (in samples).
    int blockSize;

    int currentChannel;

    // Playback options
    int currentPlayMode;
    bool isReversed;
};



#endif  // __SAMPLESTRIP_H_ACB589A6__
