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
    sampleSampleRate(0)
{

    WavAudioFormat wavFormat;
    ScopedPointer<AudioFormatReader> audioReader(wavFormat.createReaderFor(
                                                 new FileInputStream(sampleSource), true));

    int length = (int) audioReader->lengthInSamples;

    data = new AudioSampleBuffer(jmin(2, (int) audioReader->numChannels), length);
    sampleSampleRate = audioReader->sampleRate;
    data->readFromAudioReader(audioReader, 0, length + 4, 0, true, true);
}

