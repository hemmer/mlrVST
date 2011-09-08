/*
  ==============================================================================

    Zampler.cpp
    Created: 8 Sep 2011 12:27:50pm
    Author:  Hemmer

  ==============================================================================
*/


#include "Zampler.h"

//==============================================================================
ZamplerSound::ZamplerSound (const String& name_,
                            AudioFormatReader& source,
                            const BigInteger& midiNotes_,
                            const int midiNoteForNormalPitch,
                            const double attackTimeSecs,
                            const double releaseTimeSecs,
                            const double maxSampleLengthSeconds)
    : name (name_),
      midiNotes (midiNotes_),
      midiRootNote (midiNoteForNormalPitch)
{
    sourceSampleRate = source.sampleRate;

    if (sourceSampleRate <= 0 || source.lengthInSamples <= 0)
    {
        length = 0;
        attackSamples = 0;
        releaseSamples = 0;
    }
    else
    {
        length = jmin ((int) source.lengthInSamples,
                       (int) (maxSampleLengthSeconds * sourceSampleRate));

        data = new AudioSampleBuffer (jmin (2, (int) source.numChannels), length + 4);

        data->readFromAudioReader (&source, 0, length + 4, 0, true, true);

        attackSamples = roundToInt (attackTimeSecs * sourceSampleRate);
        releaseSamples = roundToInt (releaseTimeSecs * sourceSampleRate);
    }
}

ZamplerSound::~ZamplerSound()
{
}

//==============================================================================
bool ZamplerSound::appliesToNote (const int midiNoteNumber)
{
    return midiNotes [midiNoteNumber];
}

bool ZamplerSound::appliesToChannel (const int /*midiChannel*/)
{
    return true;
}


//==============================================================================
ZamplerVoice::ZamplerVoice()
    : pitchRatio (0.0),
      sourceSamplePosition (0.0),
      lgain (0.0f),
      rgain (0.0f),
      isInAttack (false),
      isInRelease (false)
{
}

ZamplerVoice::~ZamplerVoice()
{
}

bool ZamplerVoice::canPlaySound (ZynthSound* sound)
{
    return dynamic_cast <const ZamplerSound*> (sound) != nullptr;
}

void ZamplerVoice::startNote (const int midiNoteNumber,
                              const float velocity,
                              ZynthSound* s,
                              const int /*currentPitchWheelPosition*/)
{
    const ZamplerSound* const sound = dynamic_cast <const ZamplerSound*> (s);
    jassert (sound != nullptr); // this object can only play ZamplerSounds!

    if (sound != nullptr)
    {
        const double targetFreq = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        const double naturalFreq = MidiMessage::getMidiNoteInHertz (sound->midiRootNote);

        pitchRatio = (targetFreq * sound->sourceSampleRate) / (naturalFreq * getSampleRate());

        sourceSamplePosition = 0.0;
        lgain = velocity;
        rgain = velocity;

        isInAttack = (sound->attackSamples > 0);
        isInRelease = false;

        if (isInAttack)
        {
            attackReleaseLevel = 0.0f;
            attackDelta = (float) (pitchRatio / sound->attackSamples);
        }
        else
        {
            attackReleaseLevel = 1.0f;
            attackDelta = 0.0f;
        }

        if (sound->releaseSamples > 0)
        {
            releaseDelta = (float) (-pitchRatio / sound->releaseSamples);
        }
        else
        {
            releaseDelta = 0.0f;
        }
    }
}

void ZamplerVoice::stopNote (const bool allowTailOff)
{
    if (allowTailOff)
    {
        isInAttack = false;
        isInRelease = true;
    }
    else
    {
        clearCurrentNote();
    }
}

void ZamplerVoice::pitchWheelMoved (const int /*newValue*/)
{
}

void ZamplerVoice::controllerMoved (const int /*controllerNumber*/,
                                    const int /*newValue*/)
{
}

//==============================================================================
void ZamplerVoice::renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    const ZamplerSound* const playingSound = static_cast <ZamplerSound*> (getCurrentlyPlayingSound().getObject());

    if (playingSound != nullptr)
    {
        // these are the sample input data
        const float* const inL = playingSound->data->getSampleData (0, 0);
        const float* const inR = playingSound->data->getNumChannels() > 1
                                    ? playingSound->data->getSampleData (1, 0) : nullptr;

        // this is what we add the sample data to
        float* outL = outputBuffer.getSampleData (0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getSampleData (1, startSample) : nullptr;

        // for all the samples we are required to cover
        while (--numSamples >= 0)
        {
            const int pos = (int) sourceSamplePosition;
            const float alpha = (float) (sourceSamplePosition - pos);
            const float invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = (inL [pos] * invAlpha + inL [pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR [pos] * invAlpha + inR [pos + 1] * alpha)
                                       : l;

            l *= lgain;
            r *= rgain;

            if (isInAttack)
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;

                attackReleaseLevel += attackDelta;

                if (attackReleaseLevel >= 1.0f)
                {
                    attackReleaseLevel = 1.0f;
                    isInAttack = false;
                }
            }
            else if (isInRelease)
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;

                attackReleaseLevel += releaseDelta;

                if (attackReleaseLevel <= 0.0f)
                {
                    stopNote (false);
                    break;
                }
            }

            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            sourceSamplePosition += pitchRatio;

            if (sourceSamplePosition > playingSound->length)
            {
                stopNote (false);
                break;
            }
        }
    }
}


