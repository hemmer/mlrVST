/*
  ==============================================================================

    AudioSample.h
    Created: 8 Sep 2011 2:16:36pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __AUDIOSAMPLE_H_DED61AB8__
#define __AUDIOSAMPLE_H_DED61AB8__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"

class AudioSample 
{
public:
    AudioSample(const File &sampleSource);

    AudioSampleBuffer* getAudioData() const { return data; }
private:
    const File sampleFile;
    
    int sampleSampleRate;
    ScopedPointer <AudioSampleBuffer> data;
    
}


#endif  // __AUDIOSAMPLE_H_DED61AB8__
