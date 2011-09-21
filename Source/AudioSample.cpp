/*
  ==============================================================================

    AudioSample.cpp
    Created: 8 Sep 2011 2:16:36pm
    Author:  Hemmer

  ==============================================================================
*/

#include "AudioSample.h"

AudioSample::AudioSample(const File &sampleSource) :
    sampleFile(),
    data(0),
    sampleSampleRate(0.0),
    sampleLength(0),
    numChannels(0),
    sampleName(String::empty), fileType(String::empty)
{

    sampleName = sampleSource.getFileName();
    fileType = sampleSource.getFileExtension();

    ScopedPointer<AudioFormatReader> audioReader;

    if(fileType == ".wav")
    {
        WavAudioFormat wavFormat;
        audioReader = wavFormat.createReaderFor(new FileInputStream(sampleSource), true);

        // if the above failed, try loading with aiff filereader instead
        if(audioReader == 0)
        {
            AiffAudioFormat aiffFormat;
            audioReader = aiffFormat.createReaderFor(new FileInputStream(sampleSource), true);
        }
    }
    else if (fileType == ".aiff" || fileType == ".aif" || fileType == ".aifc")
    {
        AiffAudioFormat aiffFormat;
        audioReader = aiffFormat.createReaderFor(new FileInputStream(sampleSource), true);

        // if the above failed, try loading with wav filereader instead
        if(audioReader == 0)
        {
            WavAudioFormat wavFormat;
            audioReader = wavFormat.createReaderFor(new FileInputStream(sampleSource), true);
        }
    }


    // make sure we have a sucessful load
    if(audioReader != 0)
    {
        sampleLength = (int) audioReader->lengthInSamples;
        numChannels = audioReader->numChannels;

        data = new AudioSampleBuffer(jmin(2, (int) audioReader->numChannels), sampleLength);
        sampleSampleRate = audioReader->sampleRate;
        data->readFromAudioReader(audioReader, 0, sampleLength, 0, true, true);

        sampleFile = sampleSource;
    }
    else
    {
        // If we still aren't pointing to a legitimate sample,
        // fail the constructor.
        throw ("Invalid file loaded: " + sampleName);
    }
}

bool AudioSample::operator== (const AudioSample &s1) const
{
    // if each AudioSample points to the same file,
    // consider them identical
    return (getSampleFile() == s1.getSampleFile());
}