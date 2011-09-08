/*
  ==============================================================================

    AudioSample.cpp
    Created: 8 Sep 2011 2:16:36pm
    Author:  Hemmer

  ==============================================================================
*/

#include "AudioSample.h"

AudioSample::AudioSample(const File &sampleSource) :
    sampleFile(sampleSource),
    data(0),
    sampleSampleRate(0.0),
    sampleLength(0),
    numChannels(0)
{

    WavAudioFormat wavFormat;
    ScopedPointer<AudioFormatReader> audioReader(wavFormat.createReaderFor(
                                                 new FileInputStream(sampleSource), true));

    sampleLength = (int) audioReader->lengthInSamples;
    numChannels = audioReader->numChannels;

    data = new AudioSampleBuffer(jmin(2, (int) audioReader->numChannels), sampleLength);
    sampleSampleRate = audioReader->sampleRate;
    data->readFromAudioReader(audioReader, 0, sampleLength, 0, true, true);
}

