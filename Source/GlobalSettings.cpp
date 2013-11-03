/*
  ==============================================================================

    GlobalSettings.cpp
    Created: 3 Nov 2013 12:36:16pm
    Author:  hemmer

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "GlobalSettings.h"

GlobalSettings::GlobalSettings(mlrVSTAudioProcessor* owner) :
    processor(owner),

    // Misc /////////////////////////////////////////////////////////
    monomeSize(eightByEight), numMonomeRows(8), numMonomeCols(8),
    rampLength(50), numSampleStrips(7), monitorInputs(false),

    // Tempo / Quantisation /////////////////////////////////////////
    quantisationLevel(-1.0), quantiseMenuSelection(1),
    useExternalTempo(false), currentBPM(120.0),

    // Channel Setup ////////////////////////////////////////////////
    maxChannels(8), numChannels(maxChannels), masterGain(0.8f),
    channelGains(), channelMutes(), defaultChannelGain(0.8f),

    // OSC /////////////////////////////////////////////
    OSCPrefix("mlrvst"),

    // Audio / MIDI Buffers /////////////////////////////////////////
    isResampling(false), resampleLength(8), resamplePrecountLength(0),
    resampleBank(0), resampleBankSize(8),
    isRecording(false), recordLength(8), recordPrecountLength(0),
    recordBank(0), recordBankSize(8),
    currentPatternLength(8), currentPatternPrecountLength(0),
    currentPatternBank(0), patternBankSize(8)
{

}

// Static methods ///////////////////////////////////////////////
String GlobalSettings::getGlobalSettingName(const int &settingID)
{
    switch (settingID)
    {
    case sUseExternalTempo : return "use_external_tempo";
    case sPresetName : return "preset_name";
    case sNumChannels : return "num_channels";
    case sChannelGains : return "channel_gains";
    case sChannelMutes: return "channel_mutes";
    case sMonomeSize : return "monome_size";
    case sNumSampleStrips : return "num_sample_strips";
    case sMasterGain : return "master_gain";
    case sCurrentBPM : return "current_bpm";
    case sQuantiseLevel : return "quantisation_level";
    case sRampLength : return "ramp_length";
    case sQuantiseMenuSelection : return "quantize_menu";
    case sMonitorInputs : return "monitor_inputs";
    case sRecordPrecount : return "record_length";
    case sRecordLength : return "record_length";
    case sRecordBank : return "record_bank";
    case sResamplePrecount : return "resample_precount";
    case sResampleLength : return "resample_length";
    case sResampleBank : return "resample_bank";
    case sPatternPrecount : return "pattern_precount";
    case sPatternLength : return "pattern_length";
    case sPatternBank : return "pattern_bank";
    default : jassertfalse; return "name_not_found";
    }
}
int GlobalSettings::getGlobalSettingID(const String &settingName)
{
    // TODO: is this needed?
    if (settingName == "num_channels") return sNumChannels;
    if (settingName == "use_external_tempo") return sUseExternalTempo;
    if (settingName == "num_channels") return sNumChannels;
    if (settingName == "monome_size") return sMonomeSize;
    if (settingName == "num_sample_strips") return sNumSampleStrips;
    if (settingName == "current_bpm") return sCurrentBPM;
    if (settingName == "quantisation_level") return sQuantiseLevel;
    if (settingName == "ramp_length") return sRampLength;

    jassertfalse; return -1;

}
int GlobalSettings::getGlobalSettingType(const int &settingID)
{
    switch (settingID)
    {
    case sUseExternalTempo : return TypeBool;
    case sPresetName : return TypeString;
    case sNumChannels : return TypeInt;
    case sChannelGains : return TypeFloat + TypeArray;
    case sChannelMutes : return TypeBool + TypeArray;
    case sMonomeSize : return TypeInt;
    case sNumSampleStrips : return TypeInt;
    case sMasterGain : return TypeFloat;
    case sCurrentBPM : return TypeDouble;
    case sQuantiseLevel : return TypeDouble;
    case sRampLength : return TypeInt;
    case sQuantiseMenuSelection : return TypeInt;
    case sMonitorInputs : return TypeBool;
    case sRecordPrecount : return TypeInt;
    case sRecordLength : return TypeInt;
    case sRecordBank : return TypeInt;
    case sResamplePrecount : return TypeInt;
    case sResampleLength : return TypeInt;
    case sResampleBank : return TypeInt;
    case sPatternPrecount : return TypeInt;
    case sPatternLength : return TypeInt;
    case sPatternBank : return TypeInt;
    default : jassertfalse; return TypeError;
    }
}
int GlobalSettings::getSettingPresetScope(const int &settingID)
{
    // Explanation of codes:
    //  - ScopeError: we went wrong somewhere!
    //  - ScopeNone: don't save/load this setting
    //  - ScopePreset: can vary with every preset
    //  - ScopeSetlist: saved once globally

    switch (settingID)
    {
    case sUseExternalTempo : return ScopeSetlist;
    case sNumChannels : return ScopePreset;
    case sChannelGains : return ScopePreset;
    case sChannelMutes : return ScopePreset;
    case sMonomeSize : return ScopeSetlist;
    case sPresetName : return ScopePreset;
    case sNumMonomeRows : return ScopeSetlist;
    case sNumMonomeCols : return ScopeSetlist;
    case sNumSampleStrips : return ScopeSetlist;
    case sMasterGain : return ScopePreset;
    case sCurrentBPM : return ScopePreset;
    case sQuantiseLevel : return ScopePreset;
    case sQuantiseMenuSelection : return ScopePreset;
    case sOSCPrefix : return ScopeSetlist;
    case sMonitorInputs : return ScopePreset;
    case sRecordPrecount : return ScopePreset;
    case sRecordLength : return ScopePreset;
    case sRecordBank : return ScopePreset;
    case sResamplePrecount : return ScopePreset;
    case sResampleLength : return ScopePreset;
    case sResampleBank : return ScopePreset;
    case sPatternPrecount : return ScopePreset;
    case sPatternLength : return ScopePreset;
    case sPatternBank : return ScopePreset;
    case sRampLength : return ScopeSetlist;
    default : jassertfalse; return ScopeError;
    }
}

// Getters / setters //////////////////////////////////////
void GlobalSettings::setGlobalSetting(const int &settingID,
                                      const void *newValue,
                                      const bool &notifyListeners)
{
    switch (settingID)
    {
    case sUseExternalTempo :
        useExternalTempo = *static_cast<const bool*>(newValue); break;

    case sPresetName :
        presetName = *static_cast<const String*>(newValue); break;

    case sNumChannels :
		{
			numChannels = *static_cast<const int*>(newValue);
			processor->buildChannelArray();

            // reset the gains to the default
            channelGains.clear();
            channelMutes.clear();
            for (int c = 0; c < numChannels; c++)
            {
                channelGains.add(defaultChannelGain);
                channelMutes.add(false);
            }

            break;
        }

    case sMonomeSize :
        {
            monomeSize = *static_cast<const int*>(newValue);

            switch(monomeSize)
            {
            case eightByEight :     numMonomeRows = numMonomeCols = 8;
            case sixteenByEight :   numMonomeRows = 16; numMonomeCols = 8;
            case eightBySixteen :   numMonomeRows = 8; numMonomeCols = 16;
            case sixteenBySixteen : numMonomeRows = numMonomeCols = 16;
            default : jassertfalse;
            }

            processor->setupButtonStatus(numMonomeRows, numMonomeCols);
            break;
        }

    case sNumSampleStrips :
        {
            const int newNumSampleStrips = *static_cast<const int*>(newValue);

            if (newNumSampleStrips != numSampleStrips)
            {
                processor->buildSampleStripArray(newNumSampleStrips);
                numSampleStrips = newNumSampleStrips;
            }
            break;
        }

    case sOSCPrefix :
        {
            OSCPrefix = *static_cast<const String*>(newValue);
            processor->setOSCPrefix(OSCPrefix);
        }
    case sMonitorInputs :
        monitorInputs = *static_cast<const bool*>(newValue); break;

    case sResampleLength :
        resampleLength = *static_cast<const int*>(newValue); break;
    case sResamplePrecount :
        resamplePrecountLength = *static_cast<const int*>(newValue); break;
    case sResampleBank :
        resampleBank = *static_cast<const int*>(newValue); break;

    case sRecordLength :
        recordLength = *static_cast<const int*>(newValue); break;
    case sRecordPrecount :
        recordPrecountLength = *static_cast<const int*>(newValue); break;
    case sRecordBank :
        recordBank = *static_cast<const int*>(newValue); break;

    case sPatternLength :
        currentPatternLength = *static_cast<const int*>(newValue);

        // TODO: this needs fixed
        //patternRecordings[currentPatternBank]->patternLength = currentPatternLength;
        break;

    case sPatternPrecount :
        currentPatternPrecountLength = *static_cast<const int*>(newValue);

        // TODO: this needs fixed
        //patternRecordings[currentPatternBank]->patternPrecountLength = currentPatternPrecountLength;
        break;

    case sPatternBank :
        currentPatternBank = *static_cast<const int*>(newValue);

        // and load the settings associated with that bank

        // TODO: this needs fixed
        //currentPatternLength = patternRecordings[currentPatternBank]->patternLength;
        //currentPatternPrecountLength = patternRecordings[currentPatternBank]->patternPrecountLength;
        break;

    case sMasterGain :
        {
            masterGain = *static_cast<const float*>(newValue);
            break;
        }

    case sCurrentBPM :
        {
            currentBPM = *static_cast<const double*>(newValue);
            processor->changeBPM();

            break;
        }
    case sQuantiseMenuSelection :
        {
            quantiseMenuSelection = *static_cast<const int*>(newValue);

            // either quantisation is turned off
            if (quantiseMenuSelection == 1) quantisationLevel = -1.0;
            // or calculated from menu selection
            else if (quantiseMenuSelection > 1)
            {
                jassert(quantiseMenuSelection < 8);

                // menu arithmatic
                const int menuSelection = quantiseMenuSelection - 2;
                quantisationLevel = 1.0 / pow(2.0, menuSelection);
            }

            processor->updateQuantizeSettings(); break;
        }
    case sRampLength :
        {
            rampLength = *static_cast<const int*>(newValue);

            // let strips know of the change
            for (int s = 0; s < numSampleStrips; ++s)
                processor->setSampleStripParameter(SampleStrip::pRampLength, &rampLength, s, false);

            break;
        }

    default :
        jassertfalse;
    }

    // if requested, let listeners know that a global setting has changed
    if (notifyListeners) processor->sendChangeMessage();
}

void GlobalSettings::setGlobalSettingArray(const int &settingID, const int &index,
                                           const void *newValue, const bool &notifyListeners)
{
    const int settingType = getGlobalSettingType(settingID);
    const bool isArray = (settingType & TypeArray) != 0;

    if (isArray)
    {
        switch (settingID)
        {
        case sChannelGains :
            {
                jassert(index < channelGains.size());
                const float channelGain = *static_cast<const float*>(newValue);
                channelGains.set(index, channelGain);
                break;
            }
        case sChannelMutes :
            {
                jassert(index < channelMutes.size());
                const bool isMuted = *static_cast<const bool*>(newValue);
                channelMutes.set(index, isMuted);
                break;
            }
        default :
            DBG("Setting " << settingID << " not found!"); jassertfalse;
        }

        // if requested, let listeners know that a global setting has changed
        if (notifyListeners) processor->sendChangeMessage();
    }
    else
    {
        DBG("SettingID" << settingID << " is not an array!");
        jassertfalse;
    }
}

const void* GlobalSettings::getGlobalSetting(const int &settingID) const
{
    switch (settingID)
    {
    case sUseExternalTempo : return &useExternalTempo;
    case sPresetName : return &presetName;
    case sNumChannels : return &numChannels;
    case sMonomeSize : return &monomeSize;
    case sNumMonomeRows : return &numMonomeRows;
    case sNumMonomeCols : return &numMonomeCols;
    case sNumSampleStrips : return &numSampleStrips;
    case sOSCPrefix : return &OSCPrefix;
    case sMonitorInputs : return &monitorInputs;
    case sMasterGain: return &masterGain;
    case sCurrentBPM : return &currentBPM;
    case sQuantiseLevel : return &quantisationLevel;
    case sQuantiseMenuSelection : return &quantiseMenuSelection;
    case sRecordLength : return &recordLength;
    case sRecordPrecount : return &recordPrecountLength;
    case sRecordBank : return &recordBank;
    case sResampleLength : return &resampleLength;
    case sResamplePrecount : return &resamplePrecountLength;
    case sResampleBank : return &resampleBank;

    case sPatternLength : return &currentPatternLength;
    case sPatternPrecount : return &currentPatternPrecountLength;
    case sPatternBank : return &currentPatternBank;
    case sRampLength : return &rampLength;
    default : jassertfalse; return 0;
    }
}
const void* GlobalSettings::getGlobalSettingArray(const int &settingID, const int &index) const
{
    const int settingType = getGlobalSettingType(settingID);
    const bool isArray = (settingType & TypeArray) != 0;

    if (isArray)
    {
        switch (settingID)
        {
        case sChannelGains :
            {
                jassert(index < channelGains.size());
                return &(channelGains.getReference(index));
            }
        case sChannelMutes :
            {
                jassert(index < channelMutes.size());
                return &(channelMutes.getReference(index));
            }
        default :
            DBG("Setting " << settingID << " not found!"); jassertfalse; return 0;
        }
    }
    else
    {
        DBG("SettingID" << settingID << " is not an array.");
        jassertfalse;
        return 0;
    }

}
const int GlobalSettings::getGlobalSettingArrayLength(const int &settingID) const
{
    const int settingType = getGlobalSettingType(settingID);
    const bool isArray = (settingType & TypeArray) != 0;

    if (isArray)
    {
        const int settingArrayType = settingType - TypeArray;

        switch (settingID)
        {
        case sChannelGains :
        case sChannelMutes :
            return numChannels;
        default :
            DBG("SettingID " << settingID << " not found!"); jassertfalse; return 0;
        }
    }
    else
    {
        DBG("SettingID" << settingID << " is not an array.");
        jassertfalse;
        return 0;
    }


    }