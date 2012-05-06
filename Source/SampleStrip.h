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
        pStartChunk,
        pEndChunk,
        pIsVolInc,
        pIsVolDec,
        pIsPlaySpeedInc,
        pIsPlaySpeedDec,
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

    enum StopMode
    {
        mStopNormal,
        mStopTape,
        mStopInstant
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

        case pIsVolInc : return "is_vol_inc";
        case pIsVolDec : return "is_vol_dec";
        case pIsPlaySpeedInc : return "is_playspeed_inc";
        case pIsPlaySpeedDec : return "is_playspeed_dec";

        default : jassertfalse; return "parameter_not_found_" + String(parameterID);
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
        case pVisualStart : return TypeInt;
        case pVisualEnd : return TypeInt;
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
    void toggleSampleStripParam(const int &parameterID, const bool &sendChangeMsg = true);

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

private:
    // metadata //////////////
    const int sampleStripID;
    const int numSampleStrips;

    // communication //////////////////////////
    mlrVSTAudioProcessor * const parent;

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
    bool playbackStarting;  // while this is true, ramp up the volume
    float startVol;         // the volume during this ramp

    bool playbackStopping;  // while this is true, ramp down the volume
    float stopVol;          // the volume during this ramp
    int stopMode;           // do we stop normally, or with tape effect
    float tapeStopSpeed;    // the speed during the ramp (for tape mode)


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
