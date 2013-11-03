/*
  ==============================================================================

    GlobalSettings.h
    Created: 3 Nov 2013 12:36:16pm
    Author:  hemmer

    This class manages all Global settings required by the VST. This makes it
    simple to write presets / make mappings, and has the benefit of keeping the
    amount of code in PluginProcessor to a minimum.

  ==============================================================================
*/

#ifndef GLOBALSETTINGS_H_INCLUDED
#define GLOBALSETTINGS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

class GlobalSettings
{
public:

    GlobalSettings(mlrVSTAudioProcessor* owner);
    ~GlobalSettings() {};

    enum SettingsList
    {
        // List of GlobalSettings that are independent
        // the SampleStrip rows
        sUseExternalTempo,
        sPresetName,
        sNumChannels,
        sChannelGains,
        sChannelMutes,
        sMonomeSize,
        sNumMonomeRows,
        sNumMonomeCols,
        sNumSampleStrips,
        sMasterGain,
        sCurrentBPM,
        sQuantiseLevel,             // options None (-1), 1/1, 1/2, 1/4, etc
        sQuantiseMenuSelection,     // allows menu to be correctly selected
        sOSCPrefix,
        sMonitorInputs,
        sRecordPrecount,
        sRecordLength,
        sRecordBank,
        sResamplePrecount,
        sResampleLength,
        sResampleBank,
        sPatternPrecount,
        sPatternLength,
        sPatternBank,
        sRampLength,                // length of volume envelope (in samples)
        NumGlobalSettings
    };

    enum GlobalSettingScope
    {
        ScopeError = -1,    // there's been an error somwhere
        ScopeNone,          // this setting is not saved
        ScopePreset,        // this setting is saved with every preset
        ScopeSetlist        // this setting is saved once (per setlist)
    };

    enum GlobalSettingType
    {
        // These let us know what type each setting is
        // so we can programmatically write it to file.
        // NOTE: these can be chained:
        // e.g. array of Bools: TypeBool + TypeArray
        TypeError = -1,
        TypeInt = 1,
        TypeDouble = 2,
        TypeFloat = 4,
        TypeBool = 8,
        TypeString = 16,
        TypeAudioSample = 32,
        TypeArray = 64
    };




    // get the setting name (for writing in presets etc)
    static String getGlobalSettingName(const int &settingID);

    //  lookup settingID name based on name
    static int getGlobalSettingID(const String &settingName);

    // determines the setting Type, e.g. int, bool, etc
    static int getGlobalSettingType(const int &parameterID);

    // returns the scope of the setting (e.g. is it stored with every preset?)
    static int getSettingPresetScope(const int &settingID);


    // setters / getters
    void setGlobalSetting(const int &settingID, const void *newVal, const bool &notifyListeners = true);
    const void* getGlobalSetting(const int &settingID) const;

    // setters / getters (array version)
    void setGlobalSettingArray(const int &settingID, const int &index, const void *newValue, const bool &notifyListeners = true);
    const void* getGlobalSettingArray(const int &settingID, const int &index) const;
    const int getGlobalSettingArrayLength(const int &settingID) const;

    enum MonomeSizes
    {
        eightByEight = 1, eightBySixteen, sixteenByEight, sixteenBySixteen, numSizes
    };


    // TODO: these are public for each of access though should probably be private

    // Misc /////////////////////////////////////////////////////////
    // what size of device are we using (see MonomeSizes above)
    int monomeSize, numMonomeRows, numMonomeCols;

    // Sometimes when starting / stopping / looping back we
    // get a discontinuity in the stream of samples. To avoid
    // this, we apply a short volume ramp at the start / end of
    // playback and at either end of a loop point:
    int rampLength;

    int numSampleStrips;    // How many SampleStrips (roughly number of monome rows)
    bool monitorInputs;     // do we want to monitor incoming inputs?


    // Tempo / Quantisation ////////////////////////////////////////////
    // OSC messages from the monome are converted to MIDI messages.
    // quantisationLevel stores the fineness of the quantisation
    // so that 1/4 note quantisation is 0.25 etc. NOTE: any negative
    // values will be interpreted as no quantisation (-1.0 preferred).
    double quantisationLevel;
    int quantiseMenuSelection;
    bool useExternalTempo;      // either use VST hosts tempo or our own
    double currentBPM;          // tempo (in beats per minute)


    // Channel Setup ////////////////////////////////////////////////
    const int maxChannels;      // max number of channels
    int numChannels;            // how many simultaneous channels of audio do we have
    float masterGain;           // gain for combined signal (on master channel)
    Array<float> channelGains;  // individual channel volumes
    Array<bool> channelMutes;   // are we on or off
    const float defaultChannelGain;     // initial channel gain level


    // OSC /////////////////////////////////////////////
    String OSCPrefix;       // prefix for incoming / outgoing OSC messages


    // Audio / MIDI Buffers /////////////////////////////////////////

    // Resampling is taking existing mlrVST audio and recording it
    bool isResampling;
    int resampleLength, resamplePrecountLength;     // length in bars
    int resampleBank, resampleBankSize;             // which slot to use (how many are free)

    // Recording is taking external audio and recording it
    bool isRecording;
    int recordLength, recordPrecountLength;         // length in bars
    int recordBank, recordBankSize;                 // which slot to use (how many are free)

    // Patterns are collections of monome button presses that can be looped
    int currentPatternLength;
    int currentPatternPrecountLength;
    int currentPatternBank, patternBankSize;


    // this should be a Preset object?
    String presetName;

private:

    // Communication ///////////////////
    mlrVSTAudioProcessor * processor;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalSettings);

};





#endif  // GLOBALSETTINGS_H_INCLUDED
