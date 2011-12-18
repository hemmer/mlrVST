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
    AudioSample(const File &fileSampleSource,
                const int &thumbnailLength);
    AudioSample(const AudioSampleBuffer &bufferSampleSource,
                const double &sampleRate,
                const int &thumbnailLength);
    AudioSample(const double &sampleRate,
                const int &length,
                const int &thumbnailLength,
                const String &name);

    AudioSampleBuffer* getAudioData() const { return data; }

    int getSampleLength() const { return sampleLength; }
    float getSampleLengthSeconds() const { return (float) (sampleLength / sampleSampleRate); }
    int getNumChannels() const { return numChannels; }
    String getSampleName() const { return sampleName; }
    File getSampleFile() const { return sampleFile; }
    double getSampleRate() const { return sampleSampleRate; }

    // painting stuff
    void drawChannels (Graphics& g,
                       const Rectangle<int>& area,
                       float verticalZoomFactor) const;
    void drawChannel (Graphics& g,
                      const Rectangle<int>& area,
                      const int &channel,
                      float verticalZoomFactor) const;

    // allows the thumbnail to be refreshed externally
    void generateThumbnail(const int &thumbnailLength);

    // override comparison operator == 
    bool operator== (const AudioSample &s1) const;
private:
    
    // this stores the File object so we can retrieve 
    // metadata (path, comments etc)
    const File sampleFile;
    AudioFormatManager formatManager;
    String sampleName, fileType;
    
    // information about the current sample
    int sampleLength, numChannels;
    double sampleSampleRate;
    ScopedPointer <AudioSampleBuffer> data;
    //AudioSampleBuffer *data;

    // Thumbnail stuff
    OwnedArray< Array<float> > thumbnailData;
    
    bool thumbnailFinished;

    // DEBUG:
    JUCE_LEAK_DETECTOR(AudioSample);
};


#endif  // __AUDIOSAMPLE_H_DED61AB8__


