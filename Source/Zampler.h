/*
  ==============================================================================

    Zampler.h
    Created: 8 Sep 2011 12:27:50pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __ZAMPLER_H_93C283A7__
#define __ZAMPLER_H_93C283A7__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"


//==============================================================================
/**
    A subclass of SynthesiserSound that represents a sampled audio clip.

    This is based on the Sampler classes from the Juce Library but is renamed
    to the silly Zampler to avoid confusion!

    To use it, create a Synthesiser, add some ZamplerVoice objects to it, then
    give it some ZamplerSound objects to play.

    @see ZamplerVoice, Synthesiser, SynthesiserSound
*/
class JUCE_API  ZamplerSound    : public SynthesiserSound
{
public:
    //==============================================================================
    /** Creates a sampled sound from an audio reader.

        This will attempt to load the audio from the source into memory and store
        it in this object.

        @param name         a name for the sample
        @param source       the audio to load. This object can be safely deleted by the
                            caller after this constructor returns
        @param midiNotes    the set of midi keys that this sound should be played on. This
                            is used by the SynthesiserSound::appliesToNote() method
        @param midiNoteForNormalPitch   the midi note at which the sample should be played
                                        with its natural rate. All other notes will be pitched
                                        up or down relative to this one
        @param attackTimeSecs   the attack (fade-in) time, in seconds
        @param releaseTimeSecs  the decay (fade-out) time, in seconds
        @param maxSampleLengthSeconds   a maximum length of audio to read from the audio
                                        source, in seconds
    */
    ZamplerSound (const String& name,
                  AudioFormatReader& source,
                  const BigInteger& midiNotes,
                  int midiNoteForNormalPitch,
                  double attackTimeSecs,
                  double releaseTimeSecs,
                  double maxSampleLengthSeconds);

    /** Destructor. */
    ~ZamplerSound();

    //==============================================================================
    /** Returns the sample's name */
    const String& getName() const                           { return name; }

    /** Returns the audio sample data.
        This could be 0 if there was a problem loading it.
    */
    AudioSampleBuffer* getAudioData() const                 { return data; }


    //==============================================================================
    bool appliesToNote (const int midiNoteNumber);
    bool appliesToChannel (const int midiChannel);


private:
    //==============================================================================
    friend class ZamplerVoice;

    String name;
    ScopedPointer <AudioSampleBuffer> data;
    double sourceSampleRate;
    BigInteger midiNotes;
    int length, attackSamples, releaseSamples;
    int midiRootNote;

    JUCE_LEAK_DETECTOR (ZamplerSound);
};


//==============================================================================
/**
    A subclass of SynthesiserVoice that can play a ZamplerSound.

    To use it, create a Synthesiser, add some ZamplerVoice objects to it, then
    give it some SampledSound objects to play.

    @see ZamplerSound, Synthesiser, SynthesiserVoice
*/
class JUCE_API  ZamplerVoice    : public SynthesiserVoice
{
public:
    //==============================================================================
    /** Creates a ZamplerVoice.
    */
    ZamplerVoice();

    /** Destructor. */
    ~ZamplerVoice();


    //==============================================================================
    bool canPlaySound (SynthesiserSound* sound);

    void startNote (const int midiNoteNumber,
                    const float velocity,
                    SynthesiserSound* sound,
                    const int currentPitchWheelPosition);

    void stopNote (const bool allowTailOff);

    void pitchWheelMoved (const int newValue);
    void controllerMoved (const int controllerNumber,
                          const int newValue);

    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples);


private:
    //==============================================================================
    double pitchRatio;
    double sourceSamplePosition;
    float lgain, rgain, attackReleaseLevel, attackDelta, releaseDelta;
    bool isInAttack, isInRelease;

    JUCE_LEAK_DETECTOR (ZamplerVoice);
};



#endif  // __ZAMPLER_H_93C283A7__
