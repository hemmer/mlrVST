/*
  ==============================================================================

    MappingEngine.h
    Created: 11 Jul 2013 8:32:01pm
    Author:  hemmer

    Helper class to keep track of the various Mappings that
    can be made. These include:

        - SampleStrip mappings (strip volume, channel etc.)
        - PatternStrip mappings (start pattern recording etc.)
        - Global mappings (increase BPM, next preset etc.)

        - Top row mappings (these can trigger the above, plus more)

  ==============================================================================
*/

#ifndef __MAPPINGENGINE_H_326A1AC7__
#define __MAPPINGENGINE_H_326A1AC7__

#include "../JuceLibraryCode/JuceHeader.h"


class MappingEngine

{
public:

    MappingEngine();
    ~MappingEngine() {};

    // These are the possible options for mappings for
    // the top row of buttons. Several of these enable
    // a specific type of mapping to be used, e.g. load
    // a SampleStrip mapping
    enum TopRowMappings
    {
        tmNoMapping,
        // these allow mappings for SampleStrips
        // e.g. increase volume of a SampleStrip
        tmSampleStripBtnA, tmSampleStripBtnB,
        // these allow mappings for PatternStrips
        // e.g. start recording a Pattern
        tmPatternStripBtn,
        // these allow global mappings
        // e.g. next preset, prev preset
        tmGlobalMappingBtn,
        tmStartRecording,
        tmStartResampling,
        tmStopAll,
        tmTapeStopAll,

        tmNumTopRowMappings
    };

    // these are the various mapping options for SampleStrips
    // (when the appropriate modifier is held)
    enum SampleStripMappings
    {
        nmNoMapping,
        nmFindBestTempo,
        nmToggleReverse,
		nmCycleThruChannels,
        nmDecVolume,
        nmIncVolume,
        nmDecPlayspeed,
        nmIncPlayspeed,
        nmHalvePlayspeed,
        nmDoublePlayspeed,
        nmSetNormalPlayspeed,
        nmStopPlayback,
        nmStopPlaybackTape,
        nmCycleThruRecordings,
        nmCycleThruResamplings,
        nmCycleThruFileSamples,
        numNormalRowMappings
    };

    // these are the various mapping options for PatternStrips
    // (when the appropriate modifier is held)
    enum PatternStripMappings
    {
        patmapNoMapping,
        patmapStartRecording,
        patmapStopRecording,
        patmapStartPlaying,
        patmapStopPlaying,
		patmapIncLength,
		patmapDecLength,
        patmapNumMappings
    };

    // these are the various global mapping options
    // (when the appropriate modifier is held)
    enum GlobalMappings
    {
        gmNoMapping,
        gmBPMInc,
        gmBPMDec,
        gmNextPreset,
        gmPrevPreset,
        gmMasterVolInc,
        gmMasterVolDec,
        gmNumGlobalMappings
    };

    // These list the differnt types of mapping that are kept
    // track of by the MappingEngine class.
    enum RowMappingType
    {
        rmNoBtn = -1,
        rmSampleStripMappingA,
        rmSampleStripMappingB,
        rmPatternStripMapping,
        rmGlobalMapping,
        rmTopRowMapping,
        rmNumMappingButtons
    };

    // getters / setters
    const int getMonomeMapping(const int &mappingType, const int &col) const;
    void setMonomeMapping(const int &mappingType, const int &col, const int &newMapping);

    const String getMappingName(const int &mappingType, const int &mappingID) const;

private:

    // helper functions to return mapping names for each type of mapping
    const String getTopRowMappingName(const int &mappingID) const;
    const String getSampleStripMappingName(const int &mappingID) const;
    const String getPatternStripMappingName(const int &mappingID) const;
    const String getGlobalMappingName(const int &mappingID) const;

    void setupDefaultRowMappings();


    // arrays for storing mappings
    Array<int> topRowMappings;
    OwnedArray< Array<int> > sampleStripMappings;
    Array<int> patternStripMappings;
    Array<int> globalMappings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MappingEngine);

};




#endif  // __MAPPINGENGINE_H_326A1AC7__
