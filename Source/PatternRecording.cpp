/*
==============================================================================

PatternRecording.cpp
Created: 20 May 2012 12:09:22pm
Author:  Hemmer

==============================================================================
*/

#include "PluginProcessor.h"
#include "PatternRecording.h"

// forward declaration
class mlrVSTAudioProcessor;

PatternRecording::PatternRecording(mlrVSTAudioProcessor* owner, const int &idNumber) :
    slotID(idNumber),
    // MIDI //////////////////
    midiPattern(), noteOffs(),
    // Properties ////////////////////////////////////////////////////////////
    isPatternRecording(false), isPatternPlaying(false),
    isPatternStopping(false), doesPatternLoop(true),
    patternLength(8), patternPrecountLength(0),
    patternLengthInSamples(0), patternPrecountLengthInSamples(0),
    patternPosition(0), patternPrecountPosition(0),
    patternBank(0),
    // Communication /////////
    parent(owner)
{

}


void PatternRecording::recordPattern(MidiBuffer &midiMessages, const int &numSamples)
{
    // if we are recording the pattern, just rip the
    // MIDI messages from the incoming buffer
    if (isPatternRecording)
    {
        // get the iterator for the incoming buffer
        // so we can check through the messages
        MidiBuffer::Iterator i(midiMessages);
        MidiMessage message (0xf4, 0.0);
        int time;

        // if we are still during the precount, do nothing
        if (patternPrecountPosition > numSamples)
            patternPrecountPosition -= numSamples;

        // if the precount finishes during this buffer
        else if (patternPrecountPosition > 0)
        {
            // TODO: overdub could be option here?
            midiPattern.clear();

            const int numSamplesToAdd = numSamples - patternPrecountPosition;

            // DBG("#1 recording " << numSamplesToAdd << " of midi, pos: " << patternPrecountPosition);

            // get messages from the main MIDI message queue
            while(i.getNextEvent(message, time))
            {
                // and add them if they occur after the precount runs out
                if (time > patternPrecountPosition)
                {
                    midiPattern.addEvent(message, time - patternPrecountPosition);

                    // store noteOffs to fire at end
                    if (message.isNoteOn())
                    {
                        MidiMessage tempNoteOff = MidiMessage::noteOff(message.getChannel(), message.getNoteNumber(), message.getVelocity());
                        noteOffs.addEvent(tempNoteOff, 0);
                    }
                }
            }

            // the precount has finished
            patternPrecountPosition = 0;
            // we are now numSamplesToAdd into the buffer
            patternPosition = numSamplesToAdd;
        }
        else
        {
            // if we are during recording (and not near the end), just
            // add the current input into the record buffer
            if (numSamples + patternPosition < patternLengthInSamples)
            {
                // DBG("#2 recording " << numSamples << " of midi, pos: " << patternPosition);

                // get messages from the main MIDI message queue
                while(i.getNextEvent(message, time))
                {
                    midiPattern.addEvent(message, time + patternPosition);

                    // store noteOffs to fire at end
                    if (message.isNoteOn())
                    {
                        MidiMessage tempNoteOff = MidiMessage::noteOff(message.getChannel(), message.getNoteNumber(), message.getVelocity());
                        noteOffs.addEvent(tempNoteOff, 0);
                    }
                }

                //midiPattern.addEvents(midiMessages, 0, numSamples, -patternPosition);
                patternPosition += numSamples;
            }
            // otherwise we are finishing up
            else
            {
                const int numSamplesLeftToRecord = patternLengthInSamples - patternPosition;

                // add remaining messages from the main MIDI message queue
                while(i.getNextEvent(message, time))
                {
                    if (time < numSamplesLeftToRecord)
                        midiPattern.addEvent(message, time + patternPosition);

                    // store noteOffs to fire at end
                    if (message.isNoteOn())
                    {
                        MidiMessage tempNoteOff = MidiMessage::noteOff(message.getChannel(), message.getNoteNumber(), message.getVelocity());
                        noteOffs.addEvent(tempNoteOff, 0);
                    }
                }

                // add the note offs to clear any "phantom notes"
                midiPattern.addEvents(noteOffs, 0, 1, patternLengthInSamples - 1);

                // we are no longer recording
                isPatternRecording = false;

                // if we finish recording, let any listeners know
                // so they can redraw representations of the pattern
                sendChangeMessage();

                DBG("pattern " << patternBank << " finished recording.");


                // start playing back from the start straight away
                isPatternStopping = false;
                isPatternPlaying = true;
                patternPosition = 0;

                // fill the remaining buffer with the newly recorded sequence
                const int samplesRemaining = numSamples - numSamplesLeftToRecord;
                if (patternPosition + samplesRemaining < patternLengthInSamples)
                {
                    midiMessages.addEvents(midiPattern, patternPosition, samplesRemaining, numSamplesLeftToRecord);
                    patternPosition += samplesRemaining;
                }

            }

            // Let the PatternStripControl know to recache pattern
            sendChangeMessage();
        }
    }
}
void PatternRecording::playPattern(MidiBuffer &midiMessages, const int &numSamples)
{
    // if the pattern is playing, add its events to the
    // Midi message queue
    if (isPatternPlaying)
    {
        // if we are stopping
        if (isPatternStopping)
        {
            // add the note off events to the end off the buffer
            // to avoid any hanging notes
            midiMessages.addEvents(noteOffs, 0, 1, numSamples - 1);

            isPatternPlaying = false;
            isPatternStopping = false;

            // we are finished here
            return;
        }

        // calculate the correction to timesteps relative
        // to the current MIDI buffer
        const int correction = -patternPosition;

        if (patternPosition + numSamples < patternLengthInSamples)
        {
            midiMessages.addEvents(midiPattern, patternPosition, numSamples, correction);
            patternPosition += numSamples;
        }
        else
        {
            // see how many samples of the pattern we have left to play back
            const int numToAdd = patternLengthInSamples - patternPosition;
            // and add them to the main MIDI buffer
            midiMessages.addEvents(midiPattern, patternPosition, numToAdd, correction);

            // reset position
            patternPosition = 0;

            // if we are looping the pattern
            if (doesPatternLoop)
            {
                isPatternStopping = false;
                isPatternPlaying = true;

                // go back to the start of the pattern and add these samples
                // to what remains of the main MIDI buffer
                const int samplesRemainingInBuffer = numSamples - numToAdd;
                // remembering that these are offset so as to start where
                // the previously playing pattern finished
                const int offset = numToAdd;

                midiMessages.addEvents(midiPattern, patternPosition, samplesRemainingInBuffer, offset);
                patternPosition += samplesRemainingInBuffer;
            }
            else
            {
                isPatternPlaying = false;
            }

        }
    }
}

// Recording /////////////////////////////////
void PatternRecording::startPatternRecording()
{
    jassert(patternLength != 0);

    const double currentBPM = *static_cast<const double*>(parent->getGlobalSetting(mlrVSTAudioProcessor::sCurrentBPM));
    double sampleRate = parent->getSampleRate();

    patternLengthInSamples = (int) (sampleRate * (60.0 * patternLength / currentBPM));
    patternPrecountLengthInSamples = (int) (sampleRate * (60.0 * patternPrecountLength / currentBPM));

    patternPrecountPosition = patternPrecountLengthInSamples;

    // remove any previous notes from the MidiBuffers
    midiPattern.clear();
    noteOffs.clear();

    patternPosition = 0;
    isPatternRecording = true;

    // TODO: for overdub, this may change
    isPatternPlaying = false;

    DBG("pattern " + String(slotID) + " recording started");
}
void PatternRecording::stopPatternRecording()
{
    // add the note offs to clear any "phantom notes"
    midiPattern.addEvents(noteOffs, 0, 1, patternPosition);

    // we are no longer recording
    isPatternRecording = false;

    DBG("pattern " << patternBank << " stopped recording.");
}


// Playback /////////////////////////////
void PatternRecording::startPatternPlaying(const int &position)
{
    patternPosition = position;
    isPatternPlaying = true;
    isPatternStopping = false;
}
void PatternRecording::resumePatternPlaying()
{
    isPatternPlaying = true;
    isPatternStopping = false;
}
void PatternRecording::stopPatternPlaying()
{
    // signal that we are stopping
    isPatternStopping = true;
}


// Percentages ///////////////////////////
float PatternRecording::getPatternPrecountPercent() const
{
    if (patternPrecountPosition <= 0 || patternPrecountLengthInSamples <= 0 || !isPatternRecording)
        return 0.0;
    else
        return (float) (patternPrecountPosition) / (float) (patternPrecountLengthInSamples);
}
float PatternRecording::getPatternPercent() const
{
	// only continue if the pattern is playing or recording
	if (! isPatternPlaying && !isPatternRecording)
		return 0.0;
	// make sure we haven't messed up the calc
	else if (patternPosition >= patternLengthInSamples || patternLengthInSamples <= 0)
		return 0.0;
    else
        return (float) (patternPosition) / (float) (patternLengthInSamples);
}