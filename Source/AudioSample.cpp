/*
==============================================================================

AudioSample.cpp
Created: 8 Sep 2011 2:16:36pm
Author:  Hemmer

==============================================================================
*/

#include "AudioSample.h"

AudioSample::AudioSample(const File &sampleSource,
                         const int &thumbnailLength) :
    sampleFile(sampleSource),
    data(0), sampleSampleRate(0.0),
    sampleLength(0), numChannels(0),
    sampleName(sampleSource.getFileName()),
    fileType(sampleSource.getFileExtension())
{
    formatManager.registerBasicFormats();

    ScopedPointer<AudioFormatReader> audioReader;
    audioReader = formatManager.createReaderFor(new FileInputStream(sampleSource));

    // make sure we have a sucessful load
    if(audioReader != 0)
    {
        sampleLength = (int) (audioReader->lengthInSamples);

        // Can this even happen!? Better to be safe than sorry.
        if (sampleLength < 1) throw ("Zero length sample: " + sampleName);

        numChannels = audioReader->numChannels;

        data = new AudioSampleBuffer(jmin(2, (int) audioReader->numChannels), sampleLength);
        sampleSampleRate = audioReader->sampleRate;
        data->readFromAudioReader(audioReader, 0, sampleLength, 0, true, true);

        // Certain files would have a nasty click on the first/last sample,
        // so we zero that, just in case. UPDATE: this is still an issue!
        data->applyGain(0, 1, 0.0f);
        data->applyGain(sampleLength - 1, 1, 0.0f);

        
        generateThumbnail(thumbnailLength);
    }
    else
    {
        // If we still aren't pointing to a legitimate sample,
        // fail the constructor (by throwing an exception).
        throw ("Invalid file loaded: " + sampleName);
    }
}


AudioSample::AudioSample(const AudioSampleBuffer &bufferSampleSource,
                         const double &sampleRate,
                         const int &thumbnailLength) :
    sampleFile(),
    data(new AudioSampleBuffer(bufferSampleSource)),
    sampleSampleRate(sampleRate),
    sampleLength(0), numChannels(0),
    sampleName("resample"), fileType(String::empty)
{
    numChannels = bufferSampleSource.getNumChannels();
    sampleLength = bufferSampleSource.getNumSamples();

    // this shouldn't happen
    jassert(sampleLength > 0);

    generateThumbnail(thumbnailLength);
}


void AudioSample::generateThumbnail(const int &thumbnailLength)
{
    float * samplePointer = data->getSampleData(0, 0);
    float maxVal = 0.0f, minVal = 0.0f;
    int samplesPerPixel = sampleLength / thumbnailLength;
    Array<float> tempArray;

    for (int c = 0; c < numChannels; ++c)
    {
        tempArray.clear();

        for (int i = 0; i < sampleLength; ++i)
        {
            if (*samplePointer > maxVal)
                maxVal = *samplePointer;
            else if (*samplePointer < minVal)
                minVal = *samplePointer;

            if (i % samplesPerPixel == 0)
            {
                // store the max and min values sequentially
                tempArray.add(maxVal);
                tempArray.add(minVal);

                DBG(maxVal << " " << minVal);
                minVal = maxVal = 0.0f;
            }

            ++samplePointer;
        }

        // and store this channel's thumbnail
        thumbnailData.add(new Array<float>(tempArray));
    }

    DBG("Thumbnail generated for sample: " << sampleName);
}

void AudioSample::drawChannels(Graphics& g, const Rectangle<int>& area,
                               float verticalZoomFactor) const
{
    for (int i = 0; i < numChannels; ++i)
    {
        const int y1 = roundToInt ((i * area.getHeight()) / numChannels);
        const int y2 = roundToInt (((i + 1) * area.getHeight()) / numChannels);

        drawChannel (g, Rectangle<int> (area.getX(), area.getY() + y1, area.getWidth(), y2 - y1),
                     i, verticalZoomFactor);
    }
}

void AudioSample::drawChannel(Graphics& g, const Rectangle<int>& area,
                              const int &channel, float verticalZoomFactor) const 
{

    const float topY = (float) area.getY();
    const float bottomY = (float) area.getBottom();
    const float midY = (topY + bottomY) * 0.5f;
    const float vscale = verticalZoomFactor * (bottomY - topY) * 0.7f;

    float *waveform = thumbnailData[channel]->getRawDataPointer();

    // check we have a valid pointer!
    if (waveform)
    {
        float waveTop, waveBottom;

        for (int i = 0; i < thumbnailData[channel]->size() / 2; ++i)
        {
            // max and min fill alternating entries in the thumbnailData structure
            // additional factors of 0.5 are so zero samples are drawn as flat line
            waveTop = midY - *(waveform++) * vscale - 0.5f;
            waveBottom = midY - *(waveform++) * vscale + 0.5f;

            g.drawVerticalLine (i, jmax(waveTop, topY), jmin(waveBottom, bottomY));
        }
    }
}

bool AudioSample::operator== (const AudioSample &s1) const
{
    // if each AudioSample points to the same file,
    // consider them identical
    return (getSampleFile() == s1.getSampleFile());
}