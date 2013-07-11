/*
  ==============================================================================

    MappingEngine.cpp
    Created: 11 Jul 2013 8:32:01pm
    Author:  hemmer

  ==============================================================================
*/

#include "MappingEngine.h"

MappingEngine::MappingEngine() :
    // mapping arrays //////////////////////
    topRowMappings(), sampleStripMappings(),
    patternStripMappings(), globalMappings()
{
    setupDefaultRowMappings();
}

const String MappingEngine::getMappingName(const int &mappingType, const int &mappingID) const
{
    switch (mappingType)
    {
    case rmTopRowMapping : return getTopRowMappingName(mappingID);
    case rmSampleStripMappingA :
    case rmSampleStripMappingB : return getSampleStripMappingName(mappingID);
    case rmPatternStripMapping : return getPatternStripMappingName(mappingID);
    case rmGlobalMapping :return getGlobalMappingName(mappingID);
    case rmNoBtn :
    default : jassertfalse; return String::empty;
    }
}

// handle names (useful for buttons etc)
const String MappingEngine::getTopRowMappingName(const int &mappingID) const
{
    switch (mappingID)
    {
    case tmNoMapping : return "no mapping";
    case tmSampleStripBtnA : return "sample strip button A";
    case tmSampleStripBtnB : return "sample strip button B";
    case tmPatternStripBtn : return "pattern strip button";
    case tmGlobalMappingBtn : return "global mapping btn";
    case tmStartRecording : return "start recording";
    case tmStartResampling : return "stop recording";
    case tmStopAll : return "stop all strips";
    case tmTapeStopAll : return "tape stop all strips";
    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";
    }
}
const String MappingEngine::getSampleStripMappingName(const int &mappingID) const
{
    switch (mappingID)
    {
    case nmNoMapping : return "no mapping";
    case nmFindBestTempo : return "find best tempo";
    case nmToggleReverse : return "toggle reverse";
	case nmCycleThruChannels : return "cycle through channels";
    case nmDecVolume : return "decrease volume";
    case nmIncVolume : return "increase volume";
    case nmDecPlayspeed : return "decrease playspeed";
    case nmIncPlayspeed : return "increase playspeed";
    case nmHalvePlayspeed : return "/2 playspeed";
    case nmDoublePlayspeed : return "x2 playspeed";
    case nmSetNormalPlayspeed : return "set speed to 1.0";
    case nmStopPlayback : return "stop playback";
    case nmStopPlaybackTape : return "stop playback (tape)";
    case nmCycleThruRecordings : return "cycle through recordings";
    case nmCycleThruResamplings : return "cycle through resamples";
    case nmCycleThruFileSamples : return "cycle through samples";

    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";
    }
}
const String MappingEngine::getPatternStripMappingName(const int &mappingID) const
{
    switch (mappingID)
    {
    case patmapNoMapping : return "no mapping";
    case patmapStartRecording : return "start recording";
    case patmapStopRecording : return "stop recording";
    case patmapStartPlaying : return "start playing";
    case patmapStopPlaying : return "stop playing";
	case patmapDecLength : return "decrease length";
	case patmapIncLength : return "increase length";

    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";

    }
}
const String MappingEngine::getGlobalMappingName(const int &mappingID) const
{

    switch (mappingID)
    {

    case gmNoMapping : return "no mapping";
    case gmBPMInc : return "inc BPM";
    case gmBPMDec : return "dec BPM";
    case gmNextPreset : return "next preset";
    case gmPrevPreset : return "prev preset";
    case gmMasterVolInc : return "inc mstr vol";
    case gmMasterVolDec : return "dec mstr vol";

    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";

    }
}

// getters / setters
const int MappingEngine::getMonomeMapping(const int &mappingType, const int &col) const
{
    switch (mappingType)
    {
    case rmTopRowMapping : return topRowMappings[col];
    case rmSampleStripMappingA : return sampleStripMappings[0]->getUnchecked(col);
    case rmSampleStripMappingB : return sampleStripMappings[1]->getUnchecked(col);
    case rmPatternStripMapping : return patternStripMappings[col];
    case rmGlobalMapping : return globalMappings[col];
    case rmNoBtn :
    default : jassertfalse; return -1;
    }
}
void MappingEngine::setMonomeMapping(const int &mappingType, const int &col, const int &newMapping)
{
    switch (mappingType)
    {
    case rmTopRowMapping : topRowMappings.set(col, newMapping);
    case rmSampleStripMappingA : sampleStripMappings[0]->set(col, newMapping);
    case rmSampleStripMappingB : sampleStripMappings[1]->set(col, newMapping);
    case rmPatternStripMapping : return patternStripMappings.set(col, newMapping);
    case rmGlobalMapping : return patternStripMappings.set(col, newMapping);
    case rmNoBtn :
    default : jassertfalse
    }
}

// load defaults
void MappingEngine::setupDefaultRowMappings()
{
    // clear any existing mappings
    topRowMappings.clear();
    sampleStripMappings.clear();
    patternStripMappings.clear();
    globalMappings.clear();

    // add the defaults
    topRowMappings.add(tmSampleStripBtnA);
    topRowMappings.add(tmSampleStripBtnB);
    topRowMappings.add(tmPatternStripBtn);
    topRowMappings.add(tmGlobalMappingBtn);
    topRowMappings.add(tmStopAll);
    topRowMappings.add(tmTapeStopAll);
    topRowMappings.add(tmStartRecording);
    topRowMappings.add(tmStartResampling);

    Array<int> rowMappingsA;
    rowMappingsA.add(nmStopPlaybackTape);
    rowMappingsA.add(nmToggleReverse);
    rowMappingsA.add(nmDecVolume);
    rowMappingsA.add(nmIncVolume);
    rowMappingsA.add(nmDecPlayspeed);
    rowMappingsA.add(nmIncPlayspeed);
    rowMappingsA.add(nmHalvePlayspeed);
    rowMappingsA.add(nmDoublePlayspeed);

    Array<int> rowMappingsB;
    rowMappingsB.add(nmCycleThruChannels);
    rowMappingsB.add(nmCycleThruFileSamples);
    rowMappingsB.add(nmCycleThruRecordings);
    rowMappingsB.add(nmCycleThruResamplings);
    rowMappingsB.add(nmFindBestTempo);
    rowMappingsB.add(nmSetNormalPlayspeed);
    rowMappingsB.add(nmNoMapping);
    rowMappingsB.add(nmNoMapping);

    sampleStripMappings.add(new Array<int>(rowMappingsA));
    sampleStripMappings.add(new Array<int>(rowMappingsB));

    patternStripMappings.add(patmapStartRecording);
    patternStripMappings.add(patmapStopRecording);
    patternStripMappings.add(patmapStartPlaying);
    patternStripMappings.add(patmapStopPlaying);
    patternStripMappings.add(patmapDecLength);
    patternStripMappings.add(patmapIncLength);
    patternStripMappings.add(patmapNoMapping);
    patternStripMappings.add(patmapNoMapping);

    globalMappings.add(gmNextPreset);
    globalMappings.add(gmPrevPreset);
    globalMappings.add(gmBPMDec);
    globalMappings.add(gmBPMInc);
    globalMappings.add(gmMasterVolDec);
    globalMappings.add(gmMasterVolInc);
    globalMappings.add(gmNoMapping);
    globalMappings.add(gmNoMapping);
}