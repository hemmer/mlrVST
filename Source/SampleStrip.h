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
        FirstParam,                 
        ParamCurrentChannel = 0,    // This is so we can loop over params
        ParamNumChunks,
        ParamPlayMode,
        ParamIsReversed,
        ParamFractionalStart,
        ParamFractionalEnd,
        NumGUIParams,
        // The above are the only params needed by the GUI
        ParamChunkSize,
        ParamSampleStart,       // these return the key sample points
        ParamSampleEnd,         // (taking selection into account)
        ParamAudioSample,
        TotalNumParams
    };

    enum PlayMode
    { 
        LOOP = 1,               // starts the sample looping
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
        case ParamChunkSize : return "chunk size";
        case ParamPlayMode : return "playmode";
        case ParamFractionalStart : return "ParamFractionalStart";
        case ParamFractionalEnd : return "ParamFractionalEnd";
        default : return "parameter not found";
        }
    }


    //const AudioSample* getCurrentSample() const { return currentSample; }
    //void setCurrentSample(const AudioSample *newSample);


    // these are for getting / setting the int parameters
    void setSampleStripParam(const int &parameterID, const void *newValue);
    const void* getSampleStripParam(const int &parameterID) const;

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
    int chunkSize;

    int currentChannel;

    // Playback options
    int currentPlayMode;
    bool isReversed;
};



#endif  // __SAMPLESTRIP_H_ACB589A6__
