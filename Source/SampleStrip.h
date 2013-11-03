/*
  ==============================================================================

    SampleStrip.h
    Created: 13 Sep 2011 1:07:11pm
    Author:  Hemmer

    This class is the audio backend to the SampleStripControls. Each strip
    processes its own audio.

  ==============================================================================
*/

#ifndef __SAMPLESTRIP_H_ACB589A6__
#define __SAMPLESTRIP_H_ACB589A6__

#include "AudioSample.h"
#include "../JuceLibraryCode/JuceHeader.h"

// forward declaration
class mlrVSTAudioProcessor;

class SampleStrip : public ChangeBroadcaster
{
public:
    SampleStrip(const int &newID, mlrVSTAudioProcessor *owner);
    ~SampleStrip()
    {
        buttonStatus.clear();
    }

    enum SampleStripParameter
    {
        FirstParam,                     // This is so we can loop over GUI params
        pCurrentChannel = FirstParam,
        pIsPlaying,
        pPlaybackPercentage,
        pNumChunks,
        pPlayMode,
        pIsLatched,
        pIsReversed,
        pStripVolume,
        pPlaySpeed,
        pIsPlaySpeedLocked,
        pVisualStart, pVisualEnd,       // start / end points in pixels
        pAudioSample,
        NumGUIParams,
        // The above are the only params needed by the GUI

        pChunkSize = NumGUIParams,
        pSampleStart,       // these return the key sample points
        pSampleEnd,         // (taking selection into account)
        pStartChunk, pEndChunk,
        pIsVolInc, pIsVolDec,
        pIsPlaySpeedInc, pIsPlaySpeedDec,
        pFractionalStart, pFractionalEnd,
        pRampLength,
        TotalNumParams
    };

    enum SampleStripParameterType
    {
        TypeError = -1,
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


    enum StopMode
    {
        mStopNormal,
        mStopTape,
        mStopEnvelope,
        mStopInstant
    };

    // these are for getting / setting the parameters
    void setSampleStripParam(const int &parameterID, const void *newValue,
                             const bool &sendChangeMsg = true);
    const void* getSampleStripParam(const int &parameterID) const;
    void toggleSampleStripParam(const int &parameterID, const bool &sendChangeMsg = true);

	void cycleChannels();

    double findInitialPlaySpeed(const double &BPM, const float &hostSampleRate, const bool &applyChange = true);


    void updatePlaySpeedForBPMChange(const double &newBPM);
    void setBPM(const double &newBPM) { previousBPM = newBPM; }
    void updatePlaySpeedForSelectionChange();
    void modPlaySpeed(const double &factor);
    void updateCurrentPlaybackPercentage();

    void handleMidiEvent(const MidiMessage& m);
    void renderNextBlock(AudioSampleBuffer& outputBuffer, const MidiBuffer& midiData,
                         int startSample, int numSamples);
    void renderNextSection(AudioSampleBuffer& outputBuffer, int startSample,
                           int numSamples, const bool &isFollowedByNoteOn);


    void startSamplePlaying(const int &chunk);
    void stopSamplePlaying(const int &stopMode = 0);

    void setButtonStatus(const int &column, const bool &state)
    { buttonStatus.set(column, state); }

    // returns the button being held furthest left
    int getLeftmostButton() const
    {
        int leftmostButton = -1;
        for (int i = 0; i < buttonStatus.size(); ++i)
        {
            if (buttonStatus.getUnchecked(i)) { leftmostButton = i; break; }
        }
        return leftmostButton;
    }

    void printState() const
    {
        String test = " ";
        for (int i = 0; i < buttonStatus.size(); ++i)
        {
            test += (buttonStatus[i]) ? "#" : "@";
        }
        DBG(test);
    }

    static String getParameterName(const int &parameterID)
    {
        switch(parameterID)
        {
        case pCurrentChannel : return "current_channel";
        case pIsPlaying : return "is_playing";
        case pNumChunks : return "num_chunks";
        case pPlayMode : return "playmode";
        case pIsLatched : return "is_latched";
        case pIsReversed : return "is_reversed";
        case pStripVolume : return "strip_volume";
        case pPlaySpeed : return "play_speed";
        case pIsPlaySpeedLocked : return "is_play_speed_locked";

        case pChunkSize : return "chunk_size";
        case pVisualStart : return "visual_start";
        case pVisualEnd : return "visual_end";
        case pAudioSample : return "audio_sample";
        case pPlaybackPercentage : return "playback_percentage";
        case pSampleStart : return "sample_start";
        case pSampleEnd : return "sample_end";
        case pStartChunk : return "sample_chunk_start";
        case pEndChunk : return "sample_chunk_end";
        case pRampLength : return "ramp_length";

        case pIsVolInc : return "is_vol_inc";
        case pIsVolDec : return "is_vol_dec";
        case pIsPlaySpeedInc : return "is_playspeed_inc";
        case pIsPlaySpeedDec : return "is_playspeed_dec";

        case pFractionalStart : return "fractional_start";
        case pFractionalEnd : return "fractional_end";

        default : jassertfalse; return "parameter_not_found_" + String(parameterID);
        }
    }
    static int getParameterType(const int &parameterID)
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
        case pVisualStart : return TypeInt;
        case pVisualEnd : return TypeInt;
        case pAudioSample : return TypeAudioSample;
        case pPlaybackPercentage : return TypeFloat;
        case pSampleStart : return TypeInt;
        case pSampleEnd : return TypeInt;
        case pStartChunk : return TypeInt;
        case pEndChunk : return TypeInt;
        case pFractionalStart : return TypeFloat;
        case pFractionalEnd : return TypeFloat;
        default : jassertfalse; return TypeError;
        }
    }
    static int getParameterID(const String &parameterName)
    {
        // TODO: is this still needed
        if (parameterName == "current_channel") return pCurrentChannel;
        if (parameterName == "num_chunks") return pNumChunks;
        if (parameterName == "playmode") return pPlayMode;
        if (parameterName == "is_latched") return pIsLatched ;
        if (parameterName == "is_reversed") return pIsReversed;
        if (parameterName == "strip_volume") return pStripVolume;
        if (parameterName == "play_speed") return pPlaySpeed;
        if (parameterName == "is_play_speed_locked") return pIsPlaySpeedLocked;
        if (parameterName == "fractional_start") return pFractionalStart;
        if (parameterName == "fractional_end") return pFractionalEnd;

        //if (parameterName == case pChunkSize : return "chunk_size";
        //if (parameterName == case pVisualStart : return "visual_start";
        //if (parameterName == case pVisualEnd : return "visual_end";
        //if (parameterName == case pAudioSample : return "audio_sample";
        //if (parameterName == case pSampleStart : return "sample_start";
        //if (parameterName == case pSampleEnd : return "sample_end";
        //if (parameterName == case pStartChunk : return "sample_chunk_start";
        //if (parameterName == case pEndChunk : return "sample_chunk_end";


        return -1;
        jassertfalse; return -1;
    }
    static bool isParamSaved(const int &parameterID)
    {
        // here we specify whether a parameter is loaded (or saved)
        switch(parameterID)
        {
        case pCurrentChannel : return true;
        case pNumChunks : return true;
        case pPlayMode : return true;
        case pIsLatched : return true;
        case pIsReversed : return true;
        case pStripVolume : return true;
        case pPlaySpeed : return true;
        case pIsPlaySpeedLocked : return true;
        case pIsPlaying : return false;
        case pChunkSize : return false;
        case pVisualStart : return true;
        case pVisualEnd : return true;
        case pAudioSample : return true;
        case pPlaybackPercentage : return false;
        case pSampleStart : return false;
        case pSampleEnd : return false;
        case pStartChunk : return false;
        case pEndChunk : return false;
        case pIsVolInc : return false;
        case pIsVolDec : return false;
        case pIsPlaySpeedInc : return false;
        case pIsPlaySpeedDec : return false;
        case pFractionalStart : return true;
        case pFractionalEnd : return true;
        case pRampLength : return false;
        default : jassertfalse; return false;
        }
    }

    static String getPlayModeName(const int &parameterID)
    {
        switch(parameterID)
        {
        case LOOP : return "loop";
        case PLAY_CHUNK_ONCE : return "play chunk";
        case PLAY_TO_END : return "play to end";
        default : jassertfalse; return "error!";
        }
    }

private:
    // metadata //////////////
    const int sampleStripID;

    // communication //////////////////////////
    mlrVSTAudioProcessor * const parent;        // with audio processor

    // playback parameters /////////////////////
    int currentChannel;                     // what channel is selected
    float playbackPercentage;               // where in the sample we are (GUI only)
    int currentPlayMode;                    // which of the playmodes is selected
    bool isPlaying, isReversed, isLatched;  // various playback params
    double sampleCurrentPosition;           // where we are in the sample at the moment
    // start / end points (fractional, i.e. 0.5 is half way through)
    float fractionalSampleStart, fractionalSampleEnd;
    // start / end / length of the selection (in pixels)
    int visualSelectionStart, visualSelectionEnd, visualSelectionLength;
    // start / end / length / previous length of the selection (in number of samples)
    int selectionStart, selectionEnd, selectionLength, previousSelectionLength;

    const AudioSample *currentSample;   // pointer to currently selected AudioSample
    int totalSampleLength;              // length of this AudioSample in samples
    double sampleSampleRate;            // sample rate of the AudioSample (e.g. 44100Hz)

    // loop modes might only play a select part of the visual selection
    // so store the sample number which playback starts / ends
    int playbackStartPosition, playbackEndPosition;

    int loopStartChunk, loopEndChunk;   // which chunk to start / end the loop with
    int initialColumn;                  // which button started playback

    int numChunks;      // How many blocks the sample is split up into...
    int chunkSize;      // ...and what size are they (in samples).


    // Each strip has it's individual volume control: these are
    // used to (in/de)crement the strip volume using key combos
    bool volumeIncreasing, volumeDecreasing;
    float stripVolume;

    // Each strip has it's individual playspeed control: these are
    // used to (in/de)crement the strip speed using key combos
    bool playSpeedIncreasing, playSpeedDecreasing;
    double playSpeed;
    // if true, this means the playspeed is fixed even if the
    // sample selection is changed
    bool isPlaySpeedLocked;

    // use to calculate the new playspeed after bpm change
    double previousBPM;


    // starting & stopping ////////////////////////////////////////////
    int rampLength;         // length of the ramp in samples

    bool playbackStarting;  // while this is true, ramp up the volume
    float startVol;         // the volume during this ramp
    float startVolInc;      // and by how much it increases per sample
    void beginVolRampUp(const int &length);

    bool playbackStopping;  // while this is true, ramp down the volume
    float stopVol;          // the volume during this ramp
    float stopVolDec;       // and by how much it decreases per sample
    int stopMode;           // do we stop normally, or with tape effect
    float tapeStopSpeed;    // the speed during the ramp (for tape mode)
    void beginVolRampDown(const int &length);


    // misc /////////////////////////////////////////////////
    // Boolean grid which stores the status of button presses
    // where the indices correspond to the col
    Array<bool> buttonStatus;


    void updatePlayParams();
    void updateForNewSample();


    // This is used to control access to the rendering
    // callback and the note trigger methods.
    CriticalSection lock;

    JUCE_LEAK_DETECTOR(SampleStrip);
};

#endif  // __SAMPLESTRIP_H_ACB589A6__