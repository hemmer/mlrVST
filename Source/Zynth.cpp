/*
  ==============================================================================

    Zynth.cpp
    Created: 8 Sep 2011 1:43:25pm
    Author:  Hemmer

  ==============================================================================
*/



#include "Zynth.h"


//==============================================================================
ZynthSound::ZynthSound()
{
}

ZynthSound::~ZynthSound()
{
}

//==============================================================================
ZynthVoice::ZynthVoice()
    : currentSampleRate (44100.0),
      currentlyPlayingNote (-1),
      noteOnTime (0),
      keyIsDown (false),
      sostenutoPedalDown (false)
{
}

ZynthVoice::~ZynthVoice()
{
}

bool ZynthVoice::isPlayingChannel (const int midiChannel) const
{
    return currentlyPlayingSound != nullptr
            && currentlyPlayingSound->appliesToChannel (midiChannel);
}

void ZynthVoice::setCurrentPlaybackSampleRate (const double newRate)
{
    currentSampleRate = newRate;
}

void ZynthVoice::clearCurrentNote()
{
    currentlyPlayingNote = -1;
    currentlyPlayingSound = nullptr;
}

//==============================================================================
Zynth::Zynth()
    : sampleRate (0),
      lastNoteOnCounter (0),
      shouldStealNotes (true)
{
    for (int i = 0; i < numElementsInArray (lastPitchWheelValues); ++i)
        lastPitchWheelValues[i] = 0x2000;
}

Zynth::~Zynth()
{
}

//==============================================================================
ZynthVoice* Zynth::getVoice (const int index) const
{
    const ScopedLock sl (lock);
    return voices [index];
}

void Zynth::clearVoices()
{
    const ScopedLock sl (lock);
    voices.clear();
}

void Zynth::addVoice (ZynthVoice* const newVoice)
{
    const ScopedLock sl (lock);
    voices.add (newVoice);
}

void Zynth::removeVoice (const int index)
{
    const ScopedLock sl (lock);
    voices.remove (index);
}

void Zynth::clearSounds()
{
    const ScopedLock sl (lock);
    sounds.clear();
}

void Zynth::addSound (const ZynthSound::Ptr& newSound)
{
    const ScopedLock sl (lock);
    sounds.add (newSound);
}

void Zynth::removeSound (const int index)
{
    const ScopedLock sl (lock);
    sounds.remove (index);
}

void Zynth::setNoteStealingEnabled (const bool shouldStealNotes_)
{
    shouldStealNotes = shouldStealNotes_;
}

//==============================================================================
void Zynth::setCurrentPlaybackSampleRate (const double newRate)
{
    if (sampleRate != newRate)
    {
        const ScopedLock sl (lock);

        allNotesOff (0, false);

        sampleRate = newRate;

        for (int i = voices.size(); --i >= 0;)
            voices.getUnchecked (i)->setCurrentPlaybackSampleRate (newRate);
    }
}

void Zynth::renderNextBlock (AudioSampleBuffer& outputBuffer,
                                   const MidiBuffer& midiData,
                                   int startSample,
                                   int numSamples)
{
    // must set the sample rate before using this!
    jassert (sampleRate != 0);

    const ScopedLock sl (lock);

    MidiBuffer::Iterator midiIterator (midiData);
    midiIterator.setNextSamplePosition (startSample);
    MidiMessage m (0xf4, 0.0);

    while (numSamples > 0)
    {
        int midiEventPos;
        const bool useEvent = midiIterator.getNextEvent (m, midiEventPos)
                                && midiEventPos < startSample + numSamples;

        const int numThisTime = useEvent ? midiEventPos - startSample
                                         : numSamples;

        if (numThisTime > 0)
        {
            for (int i = voices.size(); --i >= 0;)
                voices.getUnchecked (i)->renderNextBlock (outputBuffer, startSample, numThisTime);
        }

        if (useEvent)
            handleMidiEvent (m);

        startSample += numThisTime;
        numSamples -= numThisTime;
    }
}

void Zynth::handleMidiEvent (const MidiMessage& m)
{
    if (m.isNoteOn())
    {
        noteOn (m.getChannel(),
                m.getNoteNumber(),
                m.getFloatVelocity());
    }
    else if (m.isNoteOff())
    {
        noteOff (m.getChannel(),
                 m.getNoteNumber(),
                 true);
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        allNotesOff (m.getChannel(), true);
    }
    else if (m.isPitchWheel())
    {
        const int channel = m.getChannel();
        const int wheelPos = m.getPitchWheelValue();
        lastPitchWheelValues [channel - 1] = wheelPos;

        handlePitchWheel (channel, wheelPos);
    }
    else if (m.isController())
    {
        handleController (m.getChannel(),
                          m.getControllerNumber(),
                          m.getControllerValue());
    }
}

//==============================================================================
void Zynth::noteOn (const int midiChannel,
                          const int midiNoteNumber,
                          const float velocity)
{
    const ScopedLock sl (lock);

    for (int i = sounds.size(); --i >= 0;)
    {
        ZynthSound* const sound = sounds.getUnchecked(i);

        if (sound->appliesToNote (midiNoteNumber)
             && sound->appliesToChannel (midiChannel))
        {
            // If hitting a note that's still ringing, stop it first (it could be
            // still playing because of the sustain or sostenuto pedal).
            for (int j = voices.size(); --j >= 0;)
            {
                ZynthVoice* const voice = voices.getUnchecked (j);

                if (voice->getCurrentlyPlayingNote() == midiNoteNumber
                     && voice->isPlayingChannel (midiChannel))
                    stopVoice (voice, true);
            }

            startVoice (findFreeVoice (sound, shouldStealNotes),
                        sound, midiChannel, midiNoteNumber, velocity);
        }
    }
}

void Zynth::startVoice (ZynthVoice* const voice,
                              ZynthSound* const sound,
                              const int midiChannel,
                              const int midiNoteNumber,
                              const float velocity)
{
    if (voice != nullptr && sound != nullptr)
    {
        if (voice->currentlyPlayingSound != nullptr)
            voice->stopNote (false);

        voice->startNote (midiNoteNumber, velocity, sound,
                          lastPitchWheelValues [midiChannel - 1]);

        voice->currentlyPlayingNote = midiNoteNumber;
        voice->noteOnTime = ++lastNoteOnCounter;
        voice->currentlyPlayingSound = sound;
        voice->keyIsDown = true;
        voice->sostenutoPedalDown = false;
    }
}

void Zynth::stopVoice (ZynthVoice* voice, const bool allowTailOff)
{
    jassert (voice != nullptr);

    voice->stopNote (allowTailOff);

    // the subclass MUST call clearCurrentNote() if it's not tailing off! RTFM for stopNote()!
    jassert (allowTailOff || (voice->getCurrentlyPlayingNote() < 0 && voice->getCurrentlyPlayingSound() == 0));
}

void Zynth::noteOff (const int midiChannel,
                           const int midiNoteNumber,
                           const bool allowTailOff)
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        ZynthVoice* const voice = voices.getUnchecked (i);

        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
        {
            ZynthSound* const sound = voice->getCurrentlyPlayingSound();

            if (sound != nullptr
                 && sound->appliesToNote (midiNoteNumber)
                 && sound->appliesToChannel (midiChannel))
            {
                voice->keyIsDown = false;

                if (! (sustainPedalsDown [midiChannel] || voice->sostenutoPedalDown))
                    stopVoice (voice, allowTailOff);
            }
        }
    }
}

void Zynth::allNotesOff (const int midiChannel, const bool allowTailOff)
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        ZynthVoice* const voice = voices.getUnchecked (i);

        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->stopNote (allowTailOff);
    }

    sustainPedalsDown.clear();
}

void Zynth::handlePitchWheel (const int midiChannel, const int wheelValue)
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        ZynthVoice* const voice = voices.getUnchecked (i);

        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->pitchWheelMoved (wheelValue);
    }
}

void Zynth::handleController (const int midiChannel,
                                    const int controllerNumber,
                                    const int controllerValue)
{
    switch (controllerNumber)
    {
        case 0x40:  handleSustainPedal   (midiChannel, controllerValue >= 64); break;
        case 0x42:  handleSostenutoPedal (midiChannel, controllerValue >= 64); break;
        case 0x43:  handleSoftPedal      (midiChannel, controllerValue >= 64); break;
        default:    break;
    }

    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        ZynthVoice* const voice = voices.getUnchecked (i);

        if (midiChannel <= 0 || voice->isPlayingChannel (midiChannel))
            voice->controllerMoved (controllerNumber, controllerValue);
    }
}

void Zynth::handleSustainPedal (int midiChannel, bool isDown)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
    const ScopedLock sl (lock);

    if (isDown)
    {
        sustainPedalsDown.setBit (midiChannel);
    }
    else
    {
        for (int i = voices.size(); --i >= 0;)
        {
            ZynthVoice* const voice = voices.getUnchecked (i);

            if (voice->isPlayingChannel (midiChannel) && ! voice->keyIsDown)
                stopVoice (voice, true);
        }

        sustainPedalsDown.clearBit (midiChannel);
    }
}

void Zynth::handleSostenutoPedal (int midiChannel, bool isDown)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
    {
        ZynthVoice* const voice = voices.getUnchecked (i);

        if (voice->isPlayingChannel (midiChannel))
        {
            if (isDown)
                voice->sostenutoPedalDown = true;
            else if (voice->sostenutoPedalDown)
                stopVoice (voice, true);
        }
    }
}

void Zynth::handleSoftPedal (int midiChannel, bool /*isDown*/)
{
    (void) midiChannel;
    jassert (midiChannel > 0 && midiChannel <= 16);
}

//==============================================================================
ZynthVoice* Zynth::findFreeVoice (ZynthSound* soundToPlay,
                                              const bool stealIfNoneAvailable) const
{
    const ScopedLock sl (lock);

    for (int i = voices.size(); --i >= 0;)
        if (voices.getUnchecked (i)->getCurrentlyPlayingNote() < 0
             && voices.getUnchecked (i)->canPlaySound (soundToPlay))
            return voices.getUnchecked (i);

    if (stealIfNoneAvailable)
    {
        // currently this just steals the one that's been playing the longest, but could be made a bit smarter..
        ZynthVoice* oldest = nullptr;

        for (int i = voices.size(); --i >= 0;)
        {
            ZynthVoice* const voice = voices.getUnchecked (i);

            if (voice->canPlaySound (soundToPlay)
                 && (oldest == nullptr || oldest->noteOnTime > voice->noteOnTime))
                oldest = voice;
        }

        jassert (oldest != nullptr);
        return oldest;
    }

    return nullptr;
}