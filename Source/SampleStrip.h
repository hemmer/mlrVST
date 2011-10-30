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
#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"

class SampleStrip : public ChangeBroadcaster
{
public:
    SampleStrip();
    ~SampleStrip()
    {
    }

    enum SampleStripParameter
    {
        FirstParam,                     // This is so we can loop over GUI params
        ParamIsPlaying = FirstParam,
        ParamCurrentChannel,        
        ParamNumChunks,
        ParamPlayMode,
        ParamIsLatched,
        ParamIsReversed,
        ParamStripVolume,
        ParamPlaySpeed,
        ParamIsPlaySpeedLocked,
        ParamPlaybackPercentage,
        ParamFractionalStart,
        ParamFractionalEnd,
        ParamAudioSample,
        NumGUIParams,
        // The above are the only params needed by the GUI
         
        ParamChunkSize = NumGUIParams,
        ParamSampleStart,       // these return the key sample points
        ParamSampleEnd,         // (taking selection into account)
        ParamStartChunk,
        ParamEndChunk,
        TotalNumParams
    };

    
    enum SampleStripParameterType
    {
        TypeInt,
        TypeDouble,
        TypeFloat,
        TypeBool,
        TypeString, 
        TypeAudioSample
    };

    enum PlayMode
    { 
        LOOP,                   // starts the sample looping
        LOOP_CHUNK,             // loops the chunk associated with a button
        PLAY_CHUNK_ONCE,        // plays the current chunk to the end
        PLAY_TO_END,            // plays from current point until the end then stops
        NUM_PLAY_MODES
    };

    String getPlayModeName(const int &parameterID) const
    {
        switch(parameterID)
        {
        case LOOP : return "loop";
        case LOOP_CHUNK : return "loop chunk";
        case PLAY_CHUNK_ONCE : return "play chunk";
        case PLAY_TO_END : return "play to end";
        default : jassertfalse; return "error!";
        }
    }

    String getParameterName(const int &parameterID) const
    {
        switch(parameterID)
        {
        case ParamCurrentChannel : return "current_channel";
        case ParamNumChunks : return "num_chunks";
        case ParamPlayMode : return "playmode";
        case ParamIsLatched : return "is_latched";
        case ParamIsReversed : return "is_reversed";
        case ParamStripVolume : return "strip_volume";
        case ParamPlaySpeed : return "play_speed";
        case ParamIsPlaySpeedLocked : return "is_play_speed_locked";
        case ParamIsPlaying : return "is_playing";
        case ParamChunkSize : return "chunk_size";
        case ParamFractionalStart : return "fractional_start";
        case ParamFractionalEnd : return "fractional_end";
        case ParamAudioSample : return "audio_sample";
        case ParamPlaybackPercentage : return "playback_percentage";
        case ParamSampleStart : return "sample_start";
        case ParamSampleEnd : return "sample_end";
        case ParamStartChunk : return "sample_chunk_start";
        case ParamEndChunk : return "sample_chunk_end";
        default : jassertfalse; return "parameter_not_found";
        }
    }

    int getParameterType(const int &parameterID) const
    {
        switch(parameterID)
        {
        case ParamCurrentChannel : return TypeInt;
        case ParamNumChunks : return TypeInt;
        case ParamPlayMode : return TypeInt;
        case ParamIsLatched : return TypeBool;
        case ParamIsReversed : return TypeBool;
        case ParamStripVolume : return TypeFloat;
        case ParamPlaySpeed : return TypeDouble;
        case ParamIsPlaySpeedLocked : return TypeBool;
        case ParamIsPlaying : return TypeBool;
        case ParamChunkSize : return TypeInt;
        case ParamFractionalStart : return TypeFloat;
        case ParamFractionalEnd : return TypeFloat;
        case ParamAudioSample : return TypeAudioSample;
        case ParamPlaybackPercentage : return TypeFloat;
        case ParamSampleStart : return TypeInt;
        case ParamSampleEnd : return TypeInt;
        case ParamStartChunk : return TypeInt;
        case ParamEndChunk : return TypeInt;
        default : jassertfalse; return -1;
        }
    }
    // these are for getting / setting the int parameters
    void setSampleStripParam(const int &parameterID,
                             const void *newValue,
                             const bool &sendChangeMsg = true);
    const void* getSampleStripParam(const int &parameterID) const;

    void updatePlaySpeedForBPMChange(const double &newBPM);
    void updatePlaySpeedForSelectionChange();
    void findInitialPlaySpeed(const double &BPM, const float &hostSampleRate);

    void modPlaySpeed(const double &factor);
private:

    // These are just so the GUI can show where in the sample we are
    bool isPlaying;
    float playbackPercentage;

    // Pointer to currently selected sample
    const AudioSample *currentSample;
    int totalSampleLength;
    double sampleSampleRate;

    // start / end points (fractional, i.e. 0.5 is half way through)
    float fractionalSampleStart, fractionalSampleEnd;
    // start / end / length of the selection (in samples)
    int selectionStart, selectionEnd, selectionLength;
    // which chunk to start / end with
    int startChunk, endChunk;
    // How many blocks the sample is split up into...
    int numChunks;
    // ...and what size are they (in samples).
    int chunkSize;

    int currentChannel;

    // Playback options
    int currentPlayMode;
    bool isReversed, isLatched;
    float stripVolume;
    double playSpeed;
    bool isPlaySpeedLocked;

    double previousBPM;
    int previousSelectionLength;
};



#endif  // __SAMPLESTRIP_H_ACB589A6__
