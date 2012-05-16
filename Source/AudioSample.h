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


class AudioSample
{
public:

    // create sample by loading file from disk
    AudioSample(const File &fileSampleSource,
                const int &thumbnailLength);

    // preallocate space for a recording / resampling type sample
    AudioSample(const double &sampleRate,
                const int &length,
                const int &thumbnailLength,
                const String &name,
                const int &newSampleType);

    ~AudioSample();

    // store the type of AudioSample (so
    // we can cycle through the banks)
    enum SampleType
    {
        tFileSample,
        tRecordedSample,
        tResampledSample,
    };

    int getSampleType() const { return sampleType; }

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

    // Files ////////////////////////////////////////
    // this stores the File object so we can retrieve
    // metadata (path, comments etc)
    const File sampleFile;
    AudioFormatManager formatManager;
    String fileType;


    // Sample information /////////////////
    int sampleLength, numChannels;
    String sampleName;
    ScopedPointer <AudioSampleBuffer> data;
    double sampleSampleRate;
    const int sampleType;


    // Thumbnails ///////////////////////////
    // low res thumbnail version of the sample
    OwnedArray< Array<float> > thumbnailData;
    // has the thumbnail finished generating
    bool thumbnailFinished;

    // DEBUG: check for leaks
    JUCE_LEAK_DETECTOR(AudioSample);
};


#endif  // __AUDIOSAMPLE_H_DED61AB8__


