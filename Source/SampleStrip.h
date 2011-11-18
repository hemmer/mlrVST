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

// forward declaration
class mlrVSTAudioProcessor;

class SampleStrip : public ChangeBroadcaster
{
public:
    SampleStrip(const int &newID,
                const int &newNumSampleStrips,
                mlrVSTAudioProcessor *owner);
    ~SampleStrip()
    {
        parent.release();
    }

    enum SampleStripParameter
    {
        FirstParam,                     // This is so we can loop over GUI params
        pIsPlaying = FirstParam,
        pCurrentChannel,        
        pNumChunks,
        pPlayMode,
        pIsLatched,
        pIsReversed,
        pStripVolume,
        pPlaySpeed,
        pIsPlaySpeedLocked,
        pPlaybackPercentage,
        pFractionalStart,
        pFractionalEnd,
        pAudioSample,
        NumGUIParams,
        // The above are the only params needed by the GUI
         
        pChunkSize = NumGUIParams,
        pSampleStart,       // these return the key sample points
        pSampleEnd,         // (taking selection into account)
        pStartChunk,
        pEndChunk,
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
        PLAY_CHUNK_ONCE,        // plays the current chunk to the end
        PLAY_TO_END,            // plays from current point until the end then stops
        NUM_PLAY_MODES
    };

    String getPlayModeName(const int &parameterID) const
    {
        switch(parameterID)
        {
        case LOOP : return "loop";
        case PLAY_CHUNK_ONCE : return "play chunk";
        case PLAY_TO_END : return "play to end";
        default : jassertfalse; return "error!";
        }
    }

    String getParameterName(const int &parameterID) const
    {
        switch(parameterID)
        {
        case pCurrentChannel : return "current_channel";
        case pNumChunks : return "num_chunks";
        case pPlayMode : return "playmode";
        case pIsLatched : return "is_latched";
        case pIsReversed : return "is_reversed";
        case pStripVolume : return "strip_volume";
        case pPlaySpeed : return "play_speed";
        case pIsPlaySpeedLocked : return "is_play_speed_locked";
        case pIsPlaying : return "is_playing";
        case pChunkSize : return "chunk_size";
        case pFractionalStart : return "fractional_start";
        case pFractionalEnd : return "fractional_end";
        case pAudioSample : return "audio_sample";
        case pPlaybackPercentage : return "playback_percentage";
        case pSampleStart : return "sample_start";
        case pSampleEnd : return "sample_end";
        case pStartChunk : return "sample_chunk_start";
        case pEndChunk : return "sample_chunk_end";
        default : jassertfalse; return "parameter_not_found";
        }
    }

    int getParameterType(const int &parameterID) const
    {
        switch(parameterID)
        {
        case pCurrentChannel : return TypeInt;
        case pNumChunks : return TypeInt;
        case pPlayMode : return TypeInt;
        case pIsLatched : return TypeBool;
        case pIsReversed : return TypeBool;
        case pStripVolume : return TypeFloat;
        case pPlaySpeed : return TypeDouble;
        case pIsPlaySpeedLocked : return TypeBool;
        case pIsPlaying : return TypeBool;
        case pChunkSize : return TypeInt;
        case pFractionalStart : return TypeFloat;
        case pFractionalEnd : return TypeFloat;
        case pAudioSample : return TypeAudioSample;
        case pPlaybackPercentage : return TypeFloat;
        case pSampleStart : return TypeInt;
        case pSampleEnd : return TypeInt;
        case pStartChunk : return TypeInt;
        case pEndChunk : return TypeInt;
        default : jassertfalse; return -1;
        }
    }
    // these are for getting / setting the int parameters
    void setSampleStripParam(const int &parameterID,
                             const void *newValue,
                             const bool &sendChangeMsg = true);
    const void* getSampleStripParam(const int &parameterID) const;


    void updatePlaySpeedForBPMChange(const double &newBPM);
    void setBPM(const double &newBPM) { previousBPM = newBPM; }
    void updatePlaySpeedForSelectionChange();
    void findInitialPlaySpeed(const double &BPM, const float &hostSampleRate);
    void modPlaySpeed(const double &factor);
    void updateCurrentPlaybackPercentage();

    void handleMidiEvent(const MidiMessage& m);
    void renderNextBlock(AudioSampleBuffer& outputBuffer,
                         const MidiBuffer& midiData,
                         int startSample, int numSamples);
    void renderNextSection(AudioSampleBuffer& outputBuffer, int startSample, int numSamples);


    void startSamplePlaying(const int &chunk);
    double stopSamplePlaying();
    

private:
    const int sampleStripID;
    const int numSampleStrips;

    ScopedPointer<mlrVSTAudioProcessor> parent;

    void updatePlayParams();

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
    // loop modes might only play a select part of that selection
    int playbackStartPosition, playbackEndPosition;


    // which chunk to start / end the loop with
    int loopStartChunk, loopEndChunk;
    // which button started playback
    int initialColumn;

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

     // where we are in the sample at the moment
    double sampleCurrentPosition;


    //==============================================================================
    /** This is used to control access to the rendering
        callback and the note trigger methods. */
    CriticalSection lock;
};



#endif  // __SAMPLESTRIP_H_ACB589A6__
