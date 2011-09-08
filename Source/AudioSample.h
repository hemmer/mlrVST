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

    int getSampleLength() const { return sampleLength; }
    int getNumChannels() const { return numChannels; }

    // TODO: isEquals / override comparison operator == 
    //friend bool operator== (AudioSample &s1, AudioSample &s2);
private:

    // TODO: should this be const?
    File sampleFile;
    
    // information about the current sample
    int sampleLength, numChannels;
    double sampleSampleRate;
    ScopedPointer <AudioSampleBuffer> data;
};

//bool operator== (AudioSample &s1, AudioSample &s2)
//{
//    return (s1.sampleFile == s2.sampleFile);
//}

#endif  // __AUDIOSAMPLE_H_DED61AB8__