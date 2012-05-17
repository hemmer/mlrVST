/*
==============================================================================

PluginProcessor.cpp

This is the main routine that handles all audio processing. The main
audio logic goes in processBlock() though this farms out individual
strips to the SampleStrip class.


GLOBAL TODO:

    - set monome dimensions as compile-time flag
    - create abstract general panel class
    - CHECK FOR THIS "operator[] returns the object by value, so creates a copy.
      When your object is a massive lump of data, you obviously never want to
      create a copy of it (and if your object is a HeapBlock it's impossible
      to copy it), so you should never use operator[], but use getReference
      instead."

==============================================================================
*/

// use this to help track down memory leaks (SLOW)
// #include <vld.h>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OSCHandler.h"
#include <cmath>

//==============================================================================
mlrVSTAudioProcessor::mlrVSTAudioProcessor() :
    // MIDI / quantisation /////////////////////////////////////////
    quantisationLevel(-1.0), quantisationOn(false),
    quantisationGap(0), quantRemaining(0), quantiseMenuSelection(1),
    quantisedBuffer(), unquantisedCollector(),
    // Sample Pools ///////////////////////////
    samplePool(), resamplePool(), recordPool(),
    // Channel Setup ////////////////////////////////////////////////
    maxChannels(8), channelGains(), channelMutes(), masterGain(0.8f),
    defaultChannelGain(0.8f), channelColours(),
    // Global Settings //////////////////////////////////////////////
    numChannels(maxChannels), useExternalTempo(false),
    currentBPM(120.0), rampLength(50),
    // Sample Strips //////////////////////
    sampleStripArray(), numSampleStrips(7),
    // OSC /////////////////////////////////////////////
    OSCPrefix("mlrvst"), oscMsgHandler(OSCPrefix, this),
    // Audio Buffers ////////////////////////////////////////////////////////////////////
    stripContrib(2, 0),
    resampleBuffer(2, 0), isResampling(false),
    resampleLength(8), resamplePrecountLength(0),
    resampleLengthInSamples(0), resamplePrecountLengthInSamples(0),
    resamplePosition(0), resamplePrecountPosition(0),
    resampleBank(0), resampleBankSize(8),

    recordBuffer(2, 0), isRecording(false),
    recordLength(8), recordPrecountLength(0),
    recordLengthInSamples(0), recordPrecountLengthInSamples(0),
    recordPosition(0), recordPrecountPosition(0),
    recordBank(0), recordBankSize(8),
    // Preset handling /////////////////////////////////////////
    presetList("PRESETLIST"), setlist("SETLIST"),
    // Mapping settings ////////////////////////////////////////
    topRowMappings(), normalRowMappings(),
    numModifierButtons(2), currentStripModifier(-1),
    // Misc /////////////////////////////////////////////////////////
    playbackLEDPosition(), monitorInputs(false)
{
    // compile time assertions
    static_jassert(THUMBNAIL_WIDTH % 8 == 0);

    // start listening for messages
    oscMsgHandler.startThread();
    DBG("OSC thread started");

    // create our SampleStrip objects
    buildSampleStripArray(numSampleStrips);
    // add our channels
    buildChannelArray(numChannels);


    setupDefaultRowMappings();

    // TODO: have actual sample rate
    for (int i = 0; i < resampleBankSize; ++i)
        resamplePool.add(new AudioSample(44100.0, 44100, THUMBNAIL_WIDTH, "resample #" + String(i), AudioSample::tResampledSample));
    for (int i = 0; i < recordBankSize; ++i)
        recordPool.add(new AudioSample(44100.0, 44100, THUMBNAIL_WIDTH, "record #" + String(i), AudioSample::tRecordedSample));

    setlist.createNewChildElement("PRESETNONE");

    lastPosInfo.resetToDefault();

    // timer for re-drawing LEDs
    const int ledRedrawIntervalInMillisecs = 100;
    startTimer(ledRedrawIntervalInMillisecs);

    // setup 2D arrays for tracking LED states
    setMonomeStatusGrids(8, 8);
}

mlrVSTAudioProcessor::~mlrVSTAudioProcessor()
{
    // stop all sample strips
    AudioSample *nullSample = 0;
    for (int s = 0; s < sampleStripArray.size(); s++)
    {
        sampleStripArray[s]->stopSamplePlaying();
        sampleStripArray[s]->setSampleStripParam(SampleStrip::pAudioSample, nullSample, false);
    }

    // stop any sort of audio
    suspendProcessing(true);

    // be polite and turn off any remaining LEDs!
    oscMsgHandler.clearGrid();

    // and sample strips
    sampleStripArray.clear(true);

    // unload samples from memory
    samplePool.clear(true);
    resamplePool.clear(true);
    recordPool.clear(true);

    DBG("Processor destructor finished.");
}

// returns the index of a file if sucessfully loaded (or pre-existing)
// returns -1 if the loading failed
int mlrVSTAudioProcessor::addNewSample(File &sampleFile)
{

    // if the sample already exists, return its (existing) index
    for(int i = 0; i < samplePool.size(); ++i)
    {
        if (samplePool[i]->getSampleFile() == sampleFile)
        {
            DBG("Sample already loaded!");
            return i;
        }
    }

    // load to load the Sample
    try{
        // TODO: add actual length here
        samplePool.add(new AudioSample(sampleFile, THUMBNAIL_WIDTH));
        DBG("Sample Loaded: " + samplePool.getLast()->getSampleName());

        // if it is sucessful, it will be the last sample in the pool
        int newSampleIndex = samplePool.size() - 1;
        return newSampleIndex;
    }
    catch(String errString)
    {
        DBG(errString);
        // return a fail code
        return -1;
    }



}

void mlrVSTAudioProcessor::setMonomeStatusGrids(const int &/*width*/, const int &height)
{
    // ignore the top row (reserved for other things)
    int effectiveHeight = height - 1;

    // reset arrays
    playbackLEDPosition.clear();

    for (int i = 0; i < effectiveHeight; ++i)
    {
        playbackLEDPosition.add(-1);
    }

    // just in case, turn off any LEDs that might have been on!
    oscMsgHandler.clearGrid();
}

void mlrVSTAudioProcessor::buildChannelArray(const int &newNumChannels)
{
    // update the number of channels
    numChannels = newNumChannels;

    // make sure all strips stop playing, and reset their channels
    int initialChannel = 0;
    for (int s = 0; s < sampleStripArray.size(); s++)
    {
        sampleStripArray[s]->stopSamplePlaying();
        sampleStripArray[s]->setSampleStripParam(SampleStrip::pCurrentChannel, &initialChannel);
    }

    // make sure we're not doing any audio processing while (re)building it
    suspendProcessing(true);


    // reset the list of channels
    channelColours.clear();
    for (int c = 0; c < numChannels; c++)
    {
        //Colour channelColour((float) (0.1f * c), 0.5f, 0.5f, 1.0f);
        Colour channelColour(c / (float) (numChannels), 0.5f, 0.5f, 1.0f);
        channelColours.add(channelColour);
    }


    // reset the gains to the default
    channelGains.clear();
    channelMutes.clear();
    for (int c = 0; c < maxChannels; c++)
    {
        channelGains.add(defaultChannelGain);
        channelMutes.add(false);
    }


    DBG("Channel processor array (re)built");

    // resume processing
    suspendProcessing(false);
}



//////////////////////
// AUDIO STUFF      //
//////////////////////
void mlrVSTAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // clear the note buffers
    unquantisedCollector.reset(sampleRate);
    quantisedBuffer.clear();

    updateQuantizeSettings();

    // this is not a completely accurate size as the block size may change with
    // time, but at least we can allocate roughly the right size:
    stripContrib.setSize(2, samplesPerBlock, false, true, false);
}

void mlrVSTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.

    quantisedBuffer.clear();
    unquantisedCollector.reset(getSampleRate());
}

void mlrVSTAudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers, etc, as it
    // means there's been a break in the audio's continuity.
}

void mlrVSTAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // store total number of samples in the audio/midi buffers
    const int numSamples = buffer.getNumSamples();


    if (!isSuspended())
    {
        // ask the host for the current time so we can display it...
        AudioPlayHead::CurrentPositionInfo newTime;

        if (useExternalTempo && getPlayHead() != 0 &&
            getPlayHead()->getCurrentPosition(newTime))
        {
            // Successfully got the current time from the host..
            lastPosInfo = newTime;
            if (currentBPM != lastPosInfo.bpm && lastPosInfo.bpm > 1.0)
            {
                // If the tempo has changed, adjust the playspeeds accordingly
                currentBPM = lastPosInfo.bpm;
                for (int s = 0; s < sampleStripArray.size(); ++s)
                    calcPlaySpeedForNewBPM(s);
            }
        }
        else
        {
            // If the host fails to fill-in the current time, we'll just clear it to a default..
            lastPosInfo.resetToDefault();
        }




        // if we are using the quantised buffer, dump all
        // queued messages at the appropriate point
        if (quantisationOn)
        {
            // if we are within range
            if (quantRemaining < numSamples)
            {
                // ...and have data
                if (!quantisedBuffer.isEmpty())
                {
                    // get the iterator for this buffer
                    MidiBuffer::Iterator i( quantisedBuffer );
                    MidiMessage message (0xf4, 0.0);
                    int time;

                    // dump any messages from it into the main MIDI message queue
                    while(i.getNextEvent(message, time))
                        midiMessages.addEvent(message, quantRemaining);

                    // empty the queue as these notes are no longer needed
                    quantisedBuffer.clear();
                }

                // finally, reset the quantisation counter
                quantRemaining += quantisationGap;
            }

            // subtract this buffer's samples from the counter
            quantRemaining -= numSamples;
        }
        else
        {
            // This adds the "live / realtime" OSC messages from the monome which
            // have been converted to midi messages (where MIDI channel is row, note
            // number is column).
            unquantisedCollector.removeNextBlockOfMessages(midiMessages, numSamples);
        }



        if (isRecording)
        {
            // if we are still during the precount, do nothing
            if (recordPrecountPosition > 0)
                // TODO: this sample count could be more accurate!
                recordPrecountPosition -= numSamples;

            else
            {
                // if we are during recording (and not near the end), just
                // add the current input into the record buffer
                if (numSamples + recordPosition < recordLengthInSamples)
                {
                    recordBuffer.addFrom(0, recordPosition, buffer, 0, 0, numSamples);
                    recordBuffer.addFrom(1, recordPosition, buffer, 1, 0, numSamples);
                    recordPosition += numSamples;
                }

                // otherwise we are finishing up and we need to store the results
                // in an AudioSample object
                else
                {
                    const int samplesToEnd = recordLengthInSamples - recordPosition;
                    // add the remaining samples
                    recordBuffer.addFrom(0, recordPosition, buffer, 0, 0, samplesToEnd);
                    recordBuffer.addFrom(1, recordPosition, buffer, 1, 0, samplesToEnd);
                    // we are no longer recording
                    isRecording = false;

                    // get the pointer to the slot in the record bank that we want to replace
                    AudioSampleBuffer *recordSlotToReplace = recordPool[recordBank]->getAudioData();
                    // and resize it in case the length has changed
                    recordSlotToReplace->setSize(2, recordLengthInSamples, false, false, false);
                    // copy in the newly recorded samples
                    recordSlotToReplace->copyFrom(0, 0, recordBuffer, 0, 0, recordLengthInSamples);
                    recordSlotToReplace->copyFrom(1, 0, recordBuffer, 1, 0, recordLengthInSamples);
                    // and update the thumbnail
                    recordPool[recordBank]->generateThumbnail(THUMBNAIL_WIDTH);

                    DBG("record slot " << recordBank << " updated.");

                    // this is just so the TimedButtons can work out that recording has finished
                    recordPosition += samplesToEnd;

                    // make sure all SampleStrips redraw to reflect new waveform
                    for (int s = 0; s < sampleStripArray.size(); ++s)
                        sampleStripArray[s]->sendChangeMessage();
                }
            }
        }

        // if we're aren't monitoring, clear any incoming audio
        if (!monitorInputs) buffer.clear();


        // make sure the buffer for SampleStrip contributions is
        // *exactly* the right size and avoid reallocating if possible
        stripContrib.setSize(2, numSamples, false, false, true);

        for (int s = 0; s < sampleStripArray.size(); s++)
        {
            sampleStripArray[s]->setBPM(currentBPM);

            // clear the contribution from the previous strip
            stripContrib.clear();
            // find this channels contribution
            sampleStripArray[s]->renderNextBlock(stripContrib, midiMessages, 0, numSamples);

            // get the associated channel so we can apply gain
            const int stripChannel = *static_cast<const int *>(sampleStripArray[s]->getSampleStripParam(SampleStrip::pCurrentChannel));

            if (!channelMutes[stripChannel])
            {
                // add this contribution scaled by the channel gain
                buffer.addFrom(0, 0, stripContrib, 0, 0, numSamples, channelGains[stripChannel]);
                buffer.addFrom(1, 0, stripContrib, 1, 0, numSamples, channelGains[stripChannel]);
            }
        }

        // Go through the outgoing data, and apply our master gain to it...
        for (int channel = 0; channel < getNumInputChannels(); ++channel)
            buffer.applyGain(channel, 0, buffer.getNumSamples(), masterGain);


        // In case we have more outputs than inputs, we'll clear any output
        // channels that didn't contain input data, (because these aren't
        // guaranteed to be empty - they may contain garbage).
        for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
            buffer.clear(i, 0, buffer.getNumSamples());


        if (isResampling)
        {
            // if we are still during the precount, do nothing
            if (resamplePrecountPosition > 0)
                // TODO: this sample count could be more accurate!
                resamplePrecountPosition -= numSamples;

            else
            {
                // if we are during recording (and not near the end), just
                // add the current input into the record buffer
                if (numSamples + resamplePosition < resampleLengthInSamples)
                {
                    resampleBuffer.addFrom(0, resamplePosition, buffer, 0, 0, numSamples);
                    resampleBuffer.addFrom(1, resamplePosition, buffer, 1, 0, numSamples);
                    resamplePosition += numSamples;
                }

                // otherwise we are finishing up and we need to store the results
                // in an AudioSample object
                else
                {
                    const int samplesToEnd = resampleLengthInSamples - resamplePosition;
                    // add the remaining samples
                    resampleBuffer.addFrom(0, resamplePosition, buffer, 0, 0, samplesToEnd);
                    resampleBuffer.addFrom(1, resamplePosition, buffer, 1, 0, samplesToEnd);
                    // we are no longer recording
                    isResampling = false;

                    // get the pointer to the slot in the resample bank that we want to replace
                    AudioSampleBuffer *resampleSlotToReplace = resamplePool[resampleBank]->getAudioData();
                    // and resize it in case the length has changed
                    resampleSlotToReplace->setSize(2, resampleLengthInSamples, false, false, false);
                    // copy in the newly recorded samples
                    resampleSlotToReplace->copyFrom(0, 0, resampleBuffer, 0, 0, resampleLengthInSamples);
                    resampleSlotToReplace->copyFrom(1, 0, resampleBuffer, 1, 0, resampleLengthInSamples);
                    // and update the thumbnail
                    resamplePool[resampleBank]->generateThumbnail(THUMBNAIL_WIDTH);

                    // this is just so buttons can work out that it's finished
                    resamplePosition += samplesToEnd;

                    DBG("resample slot " << resampleBank << " updated.");

                    // make sure all SampleStrips redraw to reflect new waveform
                    for (int s = 0; s < sampleStripArray.size(); ++s)
                        sampleStripArray[s]->sendChangeMessage();
                 }
            }
        }


    }
    else
    {
        //silence
    }
}


//////////////////////
// OSC Stuff        //
//////////////////////

void mlrVSTAudioProcessor::timerCallback()
{
    // loop over each sample strip
    for (int row = 0; row < sampleStripArray.size(); ++row)
    {

        const bool isVolInc = *static_cast<const bool*>
            (getSampleStripParameter(SampleStrip::pIsVolInc, row));
        const bool isVolDec = *static_cast<const bool*>
            (getSampleStripParameter(SampleStrip::pIsVolDec, row));

        if (isVolInc)
        {
            float stripVol = *static_cast<const float*>
                (getSampleStripParameter(SampleStrip::pStripVolume, row));

            stripVol += 0.05f;

            // stop increasing once we reach a max value
            if (stripVol > 2.0)
            {
                stripVol = 2.0;
                toggleSampleStripParameter(SampleStrip::pIsVolInc, row);
            }

            setSampleStripParameter(SampleStrip::pStripVolume, &stripVol, row);

            // finally if the modifier button is lifted, stop increasing
            if (currentStripModifier == -1) toggleSampleStripParameter(SampleStrip::pIsVolInc, row);
        }
        else if (isVolDec)
        {
            float stripVol = *static_cast<const float*>
                (getSampleStripParameter(SampleStrip::pStripVolume, row));
            stripVol -= 0.05f;

            // stop increasing once we reach a max value
            if (stripVol < 0.0)
            {
                stripVol = 0.0;
                toggleSampleStripParameter(SampleStrip::pIsVolDec, row);
            }

            setSampleStripParameter(SampleStrip::pStripVolume, &stripVol, row);

            // finally if the modifier button is lifted, stop increasing
            if (currentStripModifier == -1) toggleSampleStripParameter(SampleStrip::pIsVolDec, row);
        }


        const bool isSpeedInc = *static_cast<const bool*>
            (getSampleStripParameter(SampleStrip::pIsPlaySpeedInc, row));
        const bool isSpeedDec = *static_cast<const bool*>
            (getSampleStripParameter(SampleStrip::pIsPlaySpeedDec, row));


        if (isSpeedInc)
        {
            double stripPlaySpeed = *static_cast<const double*>
                (getSampleStripParameter(SampleStrip::pPlaySpeed, row));
            stripPlaySpeed += 0.01;
            setSampleStripParameter(SampleStrip::pPlaySpeed, &stripPlaySpeed, row);

            // finally if the modifier button is lifted, stop increasing
            if (currentStripModifier == -1) toggleSampleStripParameter(SampleStrip::pIsPlaySpeedInc, row);
        }
        else if (isSpeedDec)
        {
            double stripPlaySpeed = *static_cast<const double*>
                (getSampleStripParameter(SampleStrip::pPlaySpeed, row));
            stripPlaySpeed -= 0.01;

            if (stripPlaySpeed < 0.0)
            {
                stripPlaySpeed = 0.0;
                toggleSampleStripParameter(SampleStrip::pIsPlaySpeedDec, row);
            }

            setSampleStripParameter(SampleStrip::pPlaySpeed, &stripPlaySpeed, row);

            // finally if the modifier button is lifted, stop increasing
            if (currentStripModifier == -1) toggleSampleStripParameter(SampleStrip::pIsPlaySpeedDec, row);
        }


        // TODO: this is still a pretty horrendous way to do this
        // and will be updated to use led/row once I have it
        // working with the new OSC spec!
        const bool isPlaying = *static_cast<const bool*>
            (getSampleStripParameter(SampleStrip::pIsPlaying, row));

        if (isPlaying)
        {
            float percentage = *static_cast<const float*>
                (getSampleStripParameter(SampleStrip::pPlaybackPercentage, row));
            int numChunks = *static_cast<const int*>
                (getSampleStripParameter(SampleStrip::pNumChunks, row));

            // this is basically the x-coord of the LED to turn on
            int fractionalPosition = (int)(percentage * numChunks);

            // if the LED has changed then...
            if (playbackLEDPosition[row] != fractionalPosition)
            {
                // ...move the LED along to the new position
                oscMsgHandler.setLED(fractionalPosition, row + 1, 1);
                oscMsgHandler.setLED(playbackLEDPosition[row], row + 1, 0);
                // and store the new playback point
                playbackLEDPosition.set(row, fractionalPosition);
            }
        }
        else
        {
            // if we're not playing, make sure the row is blank
            oscMsgHandler.setRow(row + 1, 0);

            // sanity check more than anything
            playbackLEDPosition.set(row, -1);
        }
    }
}

void mlrVSTAudioProcessor::processOSCKeyPress(const int &monomeCol, const int &monomeRow, const bool &state)
{

    /* The -1 is because we are treating the second row on the device as
       the first "effective" row, the top row being reserved for other
       functions. Yes this is confusing.
    */
    const int stripID = monomeRow - 1;

    if (monomeRow == 0)
    {
        // find out the mapping for this button...
        const int colMap = getTopRowMapping(monomeCol);
        // ...and act accordingly
        switch (colMap)
        {
        case tmNoMapping : break;
        case tmModifierBtnA :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? 0 : -1;
                break;
            }
        case tmModifierBtnB :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? 1 : -1;
                break;
            }
        case tmStartRecording : startRecording(); break;
        case tmStartResampling : startResampling(); break;
        case tmStopAll : stopAllStrips(SampleStrip::mStopNormal); break;
        case tmTapeStopAll : stopAllStrips(SampleStrip::mStopTape); break;
        default : jassertfalse;
        }
    }
    else if (currentStripModifier != -1 && monomeRow <= numSampleStrips && monomeRow > 0)
    {
        // When the modifier button is held, each strip turns into a
        // set of control buttons:

        // find out the mapping for the button
        const int colMap = getNormalRowMapping(currentStripModifier, monomeCol);

        switch (colMap)
        {
        case nmNoMapping : break;
        case nmFindBestTempo :
            // TODO: next actual sample rate here!
            if (state) sampleStripArray[stripID]->findInitialPlaySpeed(currentBPM, 44100.0);
            break;

        case nmToggleReverse :
            {
                if (state) sampleStripArray[stripID]->toggleSampleStripParam(SampleStrip::pIsReversed);
                break;
            }

		case nmCycleThruChannels :
			{
				if (state) sampleStripArray[stripID]->cycleChannels();
                break;
			}
        case nmDecVolume:
            {
                const bool isVolDec = state;
                sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pIsVolDec, &isVolDec, false);
                break;
            }

        case nmIncVolume:
            {
                const bool isVolInc = state;
                sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pIsVolInc, &isVolInc, false);
                break;
            }

        case nmDecPlayspeed:
            {
                const bool isSpeedDec = state;
                sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pIsPlaySpeedDec, &isSpeedDec, false);
                break;
            }

        case nmIncPlayspeed:
            {
                const bool isSpeedInc = state;
                sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pIsPlaySpeedInc, &isSpeedInc, false);
                break;
            }

        case nmHalvePlayspeed:
            {
                if (state) sampleStripArray[stripID]->modPlaySpeed(0.5);
                break;
            }

        case nmDoublePlayspeed:
            {
                if (state) sampleStripArray[stripID]->modPlaySpeed(2.0);
                break;
            }

        case nmSetNormalPlayspeed:
            {
                const double newPlaySpeed = 1.0;
                if (state) sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pPlaySpeed, &newPlaySpeed, true);
                break;
            }

        case nmStopPlayback:
            {
                if (state) sampleStripArray[stripID]->stopSamplePlaying();
                break;
            }

        case nmStopPlaybackTape:
            {
                if (state) sampleStripArray[stripID]->stopSamplePlaying(1);
                break;
            }

        case nmCycleThruRecordings:
            {
                if (state)
                {
                    const AudioSample *currentSample = static_cast<const AudioSample*>(sampleStripArray[stripID]->getSampleStripParam(SampleStrip::pAudioSample));

                    // if the sample is not a recording, cycle to the first recorded sample
                    if (currentSample == nullptr || currentSample->getSampleType() != AudioSample::tRecordedSample)
                        sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pAudioSample, recordPool.getFirst(), true);
                    // otherwise load the next sample
                    else
                    {
                        int s = 0;
                        // find the current sample index s
                        for (; s < recordPool.size(); ++s)
                            if (recordPool[s] == currentSample) break;

                        // and load the next sample, s + 1
                        const AudioSample *nextSample = recordPool[(++s % recordPool.size())];
                        sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pAudioSample, nextSample, true);
                    }

                    // and find the optimal playspeed
                    calcInitialPlaySpeed(stripID);
                }
                break;
            }

        case nmCycleThruResamplings:
            {
                if (state)
                {
                    const AudioSample *currentSample = static_cast<const AudioSample*>(sampleStripArray[stripID]->getSampleStripParam(SampleStrip::pAudioSample));

                    // if the sample is not a resampling, cycle to the first recorded resampling
                    if (currentSample == nullptr || currentSample->getSampleType() != AudioSample::tResampledSample)
                        sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pAudioSample, resamplePool.getFirst(), true);
                    // otherwise load the next resampling
                    else
                    {
                        int s = 0;
                        // find the current sample index s
                        for (; s < resamplePool.size(); ++s)
                            if (resamplePool[s] == currentSample) break;

                        // and load the next sample, s + 1
                        const AudioSample *nextSample = resamplePool[(++s % resamplePool.size())];
                        sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pAudioSample, nextSample, true);
                    }

                    // and find the optimal playspeed
                    calcInitialPlaySpeed(stripID);
                }
                break;
            }

        case nmCycleThruFileSamples:
            {
                if (state)
                {
                    const AudioSample *currentSample = static_cast<const AudioSample*>(sampleStripArray[stripID]->getSampleStripParam(SampleStrip::pAudioSample));

                    // if the sample is not a resampling, cycle to the first recorded resampling
                    if (currentSample == nullptr || currentSample->getSampleType() != AudioSample::tFileSample)
                        sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pAudioSample, samplePool.getFirst(), true);
                    // otherwise load the next resampling
                    else
                    {
                        int s = 0;
                        // find the current sample index s
                        for (; s < samplePool.size(); ++s)
                            if (samplePool[s] == currentSample) break;

                        // and load the next sample, s + 1
                        const AudioSample *nextSample = samplePool[(++s % samplePool.size())];
                        sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pAudioSample, nextSample, true);
                    }

                    // and find the optimal playspeed
                    calcInitialPlaySpeed(stripID);
                }
                break;
            }

        // this should not happen!
        default : jassertfalse;
        }

        // The SampleStrips can get tricked into thinking a button is held
        // if it starts out as a normal press but then a modifier button is
        // pressed before the button is lifted. This corrects for this:
        if (!state) sampleStripArray[stripID]->setButtonStatus(monomeCol, state);

    }
    else if (monomeRow <= numSampleStrips && monomeRow > 0)
    {

        /* Only pass on messages that are from the allowed range of columns.
           NOTE: MIDI messages may still be passed from other sources that
           are outside this range so the channelProcessor must be aware of
           numChunks too to filter these.
        */

        const int numChunks = *static_cast<const int*>
            (sampleStripArray[stripID]->getSampleStripParam(SampleStrip::pNumChunks));

        if (monomeCol < numChunks)
        {

            // button pressed down
            if (state)
            {
                if (quantisationOn)
                {
                    // If we are quantising, create an on message and store it it the queue -
                    // then once the quantised interval (of length quantisationGap) has
                    // elapsed (i.e. quantisation remaining < 0) we can fire in all the messages
                    // at once!
                    MidiMessage quantisedMessage(MidiMessage::noteOn(monomeRow, monomeCol, 1.0f), 0.01);
                    quantisedBuffer.addEvent(quantisedMessage, 0);
                    // DBG("OSC quantising NOTE ON: (" << stripID << ", " << monomeCol << ")");
                }
                // The implicit +1 here is because midi channels start at 1 not 0!
                else
                {
                    MidiMessage unquantisedMessage(MidiMessage::noteOn(monomeRow, monomeCol, 1.0f), 0.01);
                    const double stamp = juce::Time::getMillisecondCounterHiRes() / 1000.0;
                    unquantisedMessage.setTimeStamp(stamp);
                    unquantisedCollector.addMessageToQueue(unquantisedMessage);
                }
            }

            // button released
            else
            {
                if (quantisationOn)
                {
                    MidiMessage quantisedMessage(MidiMessage::noteOff(monomeRow, monomeCol), 0.01);
                    quantisedBuffer.addEvent(quantisedMessage, 0);
                    // DBG("OSC quantising NOTE OFF: (" << stripID << ", " << monomeCol << ")");
                }
                else
                {
                    MidiMessage unquantisedMessage(MidiMessage::noteOff(monomeRow, monomeCol), 0.01);
                    const double stamp = juce::Time::getMillisecondCounterHiRes() / 1000.0;
                    unquantisedMessage.setTimeStamp(stamp);
                    unquantisedCollector.addMessageToQueue(unquantisedMessage);
                }
            }

        }

    }
}


///////////////////////
// SampleStrip stuff //
///////////////////////
void mlrVSTAudioProcessor::buildSampleStripArray(const int &newNumSampleStrips)
{
    // make sure we're not using the sampleStripArray while (re)building it
    suspendProcessing(true);


    sampleStripArray.clear();
    numSampleStrips = newNumSampleStrips;

    for (int i = 0; i < numSampleStrips; ++i)
    {
        sampleStripArray.add(new SampleStrip(i, numSampleStrips, this));
    }

    DBG("SampleStrip array built");

    // resume processing
    suspendProcessing(false);
}
void mlrVSTAudioProcessor::calcInitialPlaySpeed(const int &stripID)
{
    // TODO insert proper host speed here
    sampleStripArray[stripID]->findInitialPlaySpeed(currentBPM, 44100.0);
}
void mlrVSTAudioProcessor::calcPlaySpeedForNewBPM(const int &stripID)
{
    sampleStripArray[stripID]->updatePlaySpeedForBPMChange(currentBPM);
}
void mlrVSTAudioProcessor::calcPlaySpeedForSelectionChange(const int &stripID)
{
    sampleStripArray[stripID]->updatePlaySpeedForSelectionChange();
}
void mlrVSTAudioProcessor::modPlaySpeed(const double &factor, const int &stripID)
{
    sampleStripArray[stripID]->modPlaySpeed(factor);
}

AudioSample * mlrVSTAudioProcessor::getAudioSample(const int &samplePoolIndex, const int &poolID)
{
    switch (poolID)
    {
    case pSamplePool:
        {
            if (samplePoolIndex >= 0 && samplePoolIndex < samplePool.size())
                return samplePool[samplePoolIndex];
            else return 0;
        }
    case pResamplePool :
        {
            if (samplePoolIndex >= 0 && samplePoolIndex < resamplePool.size())
                return resamplePool[samplePoolIndex];
            else return 0;
        }
    case pRecordPool :
        {
            if (samplePoolIndex >= 0 && samplePoolIndex < recordPool.size())
                return recordPool[samplePoolIndex];
            else return 0;
        }
    default :
        jassertfalse;
        return 0;
    }
}
SampleStrip* mlrVSTAudioProcessor::getSampleStrip(const int &index)
{
    jassert( index < sampleStripArray.size() );
    SampleStrip *tempStrip = sampleStripArray[index];
    return tempStrip;
}

void mlrVSTAudioProcessor::switchChannels(const int &newChan, const int &stripID)
{
    // Let the strip now about the new channel
    sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pCurrentChannel, &newChan);
}

void mlrVSTAudioProcessor::stopAllStrips(const int &stopMode)
{
    for (int s = 0; s < sampleStripArray.size(); ++s)
    {
        sampleStripArray[s]->stopSamplePlaying(stopMode);
    }
}

/////////////////////
// Preset Handling //
/////////////////////

void mlrVSTAudioProcessor::savePreset(const String &presetName)
{
    /* TODO LIST:

        - save global settings with each preset
        - have button that saves current global settings to all presets

    */

    // Create an outer XML element..
    XmlElement newPreset("PRESET");

    newPreset.setAttribute("name", presetName);

    ///////////////////////////
    /// TODO
    // - make sure store fractional start not visual

    // Store the SampleStrip specific settings
    for (int strip = 0; strip < sampleStripArray.size(); ++strip)
    {
        XmlElement *stripXml = new XmlElement("STRIP");
        stripXml->setAttribute("id", strip);

        // write all parameters to XML
        for (int param = 0; param < SampleStrip::NumGUIParams; ++param)
        {
            int paramType = sampleStripArray[strip]->getParameterType(param);
            String paramName = sampleStripArray[strip]->getParameterName(param);

            const void *p = sampleStripArray[strip]->getSampleStripParam(param);

            switch (paramType)
            {
            case SampleStrip::TypeBool :
                stripXml->setAttribute(paramName, (int)(*static_cast<const bool*>(p)));
                break;

            case SampleStrip::TypeInt :
                stripXml->setAttribute(paramName, *static_cast<const int*>(p));
                break;

            case SampleStrip::TypeDouble :
                stripXml->setAttribute(paramName, *static_cast<const double*>(p));
                break;

            case SampleStrip::TypeFloat :
                stripXml->setAttribute(paramName, (double)(*static_cast<const float*>(p)));
                break;

            case SampleStrip::TypeAudioSample :
                {
                    const AudioSample * sample = static_cast<const AudioSample*>(p);
                    if (sample)
                    {
                        String samplePath = sample->getSampleFile().getFullPathName();
                        stripXml->setAttribute(paramName, samplePath);
                    }
                    break;
                }
            default : jassertfalse;
            }
        }

        newPreset.addChildElement(stripXml);
    }


    // See if a preset of this name exists in the set list
    XmlElement* p = presetList.getFirstChildElement();
    bool duplicateFound = false;

    while (p != nullptr)
    {
        String pName = p->getStringAttribute("name");

        // If it does, replace it
        if (pName == presetName)
        {
            presetList.replaceChildElement(p, new XmlElement(newPreset));
            duplicateFound = true;
            break;
        }
        else
            p = p->getNextElement();
    }

    // Otherwise, add the new preset
    if (!duplicateFound)
        presetList.addChildElement(new XmlElement(newPreset));

    DBG(presetList.createDocument(String::empty));
}

void mlrVSTAudioProcessor::switchPreset(const int &id)
{
    // get the preset at the specified index in the setlist
    XmlElement* preset = setlist.getChildElement(id);

    // this *should* exist!
    if (preset)
    {
        if (preset->getTagName() == "PRESETNONE")
        {
            // we have a blank preset
            DBG("blank preset loaded");
        }
        else
        {
            // check we know the name
            DBG("preset \"" << preset->getStringAttribute("name") << "\" loaded.");

            // TODO: first load the global parameters


            // then process each strip
            XmlElement* strip = preset->getFirstChildElement();
            while(strip != nullptr)
            {

                // load the next strip
                strip = strip->getNextElement();
            }
        }
    }
    else jassertfalse;
}


////////////////////////////
// RECORDING / RESAMPLING //
////////////////////////////

void mlrVSTAudioProcessor::startResampling()
{
    resampleLengthInSamples = (int) (getSampleRate() * (60.0 * resampleLength / currentBPM));
    resamplePrecountLengthInSamples = (int) (getSampleRate() * (60.0 * resamplePrecountLength / currentBPM));
    resamplePrecountPosition = resamplePrecountLengthInSamples;

    // TODO: try writing directly into the bank
    //resamplePool[resampleBank]->
    //resampleBuffer = resamplePool[resampleBank]->getAudioData();

    resampleBuffer.setSize(2, resampleLengthInSamples, false, true);
    resampleBuffer.clear();
    resamplePosition = 0;
    isResampling = true;

    DBG("resampling started");
}

void mlrVSTAudioProcessor::startRecording()
{
    recordLengthInSamples = (int) (getSampleRate() * (60.0 * recordLength / currentBPM));
    recordPrecountLengthInSamples = (int) (getSampleRate() * (60.0 * recordPrecountLength / currentBPM));
    recordPrecountPosition = recordPrecountLengthInSamples;

    recordBuffer.setSize(2, recordLengthInSamples, false, true);
    recordBuffer.clear();
    recordPosition = 0;
    isRecording = true;

    DBG("recording started");
}



/////////////////////////////
// GETTERS AND SETTERS etc //
/////////////////////////////
AudioProcessorEditor* mlrVSTAudioProcessor::createEditor()
{
    return new mlrVSTAudioProcessorEditor(this, numChannels, numSampleStrips);
}

void mlrVSTAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // This method stores parameters in the memory block

    // Create an outer XML element..
    XmlElement xml("GLOBALSETTINGS");

    // add some attributes to it..

    xml.setAttribute("MASTER_GAIN", masterGain);
    for (int c = 0; c < channelGains.size(); c++)
    {
        String name("CHANNEL_GAIN");
        name += String(c);
        xml.setAttribute(name, channelGains[c]);
    }

    xml.setAttribute("CURRENT_PRESET", "none");

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary(xml, destData);
}
void mlrVSTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != 0)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName("GLOBALSETTINGS"))
        {
            // ok, now pull out our parameters...
            masterGain  = (float) xmlState->getDoubleAttribute("MASTER_GAIN", masterGain);
            for (int c = 0; c < channelGains.size(); c++)
            {
                String name("CHANNEL_GAIN");
                name += String(c);
                channelGains.set(c, (float) xmlState->getDoubleAttribute(name, channelGains[c]));
            }
        }
    }
}

const String mlrVSTAudioProcessor::getInputChannelName(const int channelIndex) const
{
    return String(channelIndex + 1);
}
const String mlrVSTAudioProcessor::getOutputChannelName(const int channelIndex) const
{
    return String(channelIndex + 1);
}

bool mlrVSTAudioProcessor::isInputChannelStereoPair(int /*index*/) const
{
    return true;
}
bool mlrVSTAudioProcessor::isOutputChannelStereoPair(int /*index*/) const
{
    return true;
}

bool mlrVSTAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}
bool mlrVSTAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

int mlrVSTAudioProcessor::getNumParameters()
{
    return totalNumParams;
}
float mlrVSTAudioProcessor::getParameter(int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case pMasterGainParam:
            return masterGain;
        default:            return 0.0f;
    }
}
void mlrVSTAudioProcessor::setParameter(int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case pMasterGainParam :
            masterGain = newValue;
            break;
        default: break;
    }
}
const String mlrVSTAudioProcessor::getParameterName(int index)
{
    switch (index)
    {
        case pMasterGainParam: return "master gain";
        default:            break;
    }

    return String::empty;
}
const String mlrVSTAudioProcessor::getParameterText(int index)
{
    return String(getParameter(index), 2);
}

void mlrVSTAudioProcessor::updateGlobalSetting(const int &settingID, const void *newValue)
{
    switch (settingID)
    {
    case sUseExternalTempo :
        useExternalTempo = *static_cast<const bool*>(newValue); break;
    case sNumChannels :
        numChannels = *static_cast<const int*>(newValue); break;
    case sOSCPrefix :
        {
            OSCPrefix = *static_cast<const String*>(newValue);
            oscMsgHandler.setPrefix(OSCPrefix); break;
        }
    case sMonitorInputs :
        monitorInputs = *static_cast<const bool*>(newValue); break;

    case sResampleLength :
        resampleLength = *static_cast<const int*>(newValue); break;
    case sResamplePrecount :
        resamplePrecountLength = *static_cast<const int*>(newValue); break;
    case sResampleBank :
        resampleBank = *static_cast<const int*>(newValue); break;

    case sRecordLength :
        recordLength = *static_cast<const int*>(newValue); break;
    case sRecordPrecount :
        recordPrecountLength = *static_cast<const int*>(newValue); break;
    case sRecordBank :
        recordBank = *static_cast<const int*>(newValue); break;

    case sCurrentBPM :
        {
            currentBPM = *static_cast<const double*>(newValue);

            updateQuantizeSettings();
            for (int s = 0; s < sampleStripArray.size(); ++s)
                calcPlaySpeedForNewBPM(s);
            break;
        }
    case sQuantiseMenuSelection :
        {
            quantiseMenuSelection = *static_cast<const int*>(newValue);

            // either quantisation is turned off
            if (quantiseMenuSelection == 1) quantisationLevel = -1.0;
            // or calculated from menu selection
            else if (quantiseMenuSelection > 1)
            {
                jassert(quantiseMenuSelection < 8);

                // menu arithmatic
                const int menuSelection = quantiseMenuSelection - 2;
                quantisationLevel = 1.0 / pow(2.0, menuSelection);
            }
            updateQuantizeSettings(); break;
        }
    case sRampLength :
        {
            rampLength = *static_cast<const int*>(newValue);

            // let strips know of the change
            for (int s = 0; s < sampleStripArray.size(); ++s)
                sampleStripArray[s]->setSampleStripParam(SampleStrip::pRampLength, &rampLength, false);

            break;
        }

    default :
        jassertfalse;
    }
}
const void* mlrVSTAudioProcessor::getGlobalSetting(const int &settingID) const
{
    switch (settingID)
    {
    case sUseExternalTempo : return &useExternalTempo;
    case sNumChannels : return &numChannels;
    case sOSCPrefix : return &OSCPrefix;
    case sMonitorInputs : return &monitorInputs;
    case sCurrentBPM : return &currentBPM;
    case sQuantiseLevel : return &quantisationLevel;
    case sQuantiseMenuSelection : return &quantiseMenuSelection;
    case sRecordLength : return &recordLength;
    case sRecordPrecount : return &recordPrecountLength;
    case sRecordBank : return &recordBank;
    case sResampleLength : return &resampleLength;
    case sResamplePrecount : return &resamplePrecountLength;
    case sResampleBank : return &resampleBank;
    case sRampLength : return &rampLength;
    default : jassertfalse; return 0;
    }
}
String mlrVSTAudioProcessor::getGlobalSettingName(const int &settingID) const
{
    switch (settingID)
    {
    case sUseExternalTempo : return "use_external_tempo";
    case sNumChannels : return "num_channels";
    case sCurrentBPM : return "current_bpm";
    case sQuantiseLevel : return "quantisation_level";
    case sRampLength : return "ramp_length";
    default : jassertfalse; return "name_not_found";
    }
}

String mlrVSTAudioProcessor::getTopRowMappingName(const int &mappingID)
{
    switch (mappingID)
    {
    case tmNoMapping : return "no mapping";
    case tmModifierBtnA : return "modifier button A";
    case tmModifierBtnB : return "modifier button B";
    case tmStartRecording : return "start recording";
    case tmStartResampling : return "stop recording";
    case tmStopAll : return "stop all strips";
    case tmTapeStopAll : return "tape stop all strips";
    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";
    }
}
String mlrVSTAudioProcessor::getNormalRowMappingName(const int &mappingID)
{
    switch (mappingID)
    {
    case nmNoMapping : return "no mapping";
    case nmFindBestTempo : return "find best tempo";
    case nmToggleReverse : return "toggle reverse";
	case nmCycleThruChannels : return "cycle through channels";
    case nmDecVolume : return "decrease volume";
    case nmIncVolume : return "increase volume";
    case nmDecPlayspeed : return "decrease playspeed";
    case nmIncPlayspeed : return "increase playspeed";
    case nmHalvePlayspeed : return "/2 playspeed";
    case nmDoublePlayspeed : return "x2 playspeed";
    case nmSetNormalPlayspeed : return "set playspeed to 1.0 (normal speed)";
    case nmStopPlayback : return "stop playback";
    case nmStopPlaybackTape : return "stop playback (tape)";
    case nmCycleThruRecordings : return "cycle through recorded samples";
    case nmCycleThruResamplings : return "cycle through resampled samples";
    case nmCycleThruFileSamples : return "cycle through sample files";

    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";
    }
}

void mlrVSTAudioProcessor::setupDefaultRowMappings()
{
    // clear any existing mappings
    topRowMappings.clear();
    normalRowMappings.clear();

    // add the defaults
    topRowMappings.add(tmModifierBtnA);
    topRowMappings.add(tmModifierBtnB);
    topRowMappings.add(tmStopAll);
    topRowMappings.add(tmTapeStopAll);
    topRowMappings.add(tmNoMapping);
    topRowMappings.add(tmNoMapping);
    topRowMappings.add(tmStartRecording);
    topRowMappings.add(tmStartResampling);

    Array<int> rowMappingsA;
    rowMappingsA.add(nmStopPlaybackTape);
    rowMappingsA.add(nmToggleReverse);
    rowMappingsA.add(nmDecVolume);
    rowMappingsA.add(nmIncVolume);
    rowMappingsA.add(nmDecPlayspeed);
    rowMappingsA.add(nmIncPlayspeed);
    rowMappingsA.add(nmHalvePlayspeed);
    rowMappingsA.add(nmDoublePlayspeed);

    Array<int> rowMappingsB;
	rowMappingsB.add(nmCycleThruChannels);
    rowMappingsB.add(nmCycleThruFileSamples);
    rowMappingsB.add(nmCycleThruRecordings);
    rowMappingsB.add(nmCycleThruResamplings);
	rowMappingsB.add(nmFindBestTempo);
    rowMappingsB.add(nmSetNormalPlayspeed);
	rowMappingsB.add(nmNoMapping);
    rowMappingsB.add(nmNoMapping);

    normalRowMappings.add(new Array<int>(rowMappingsA));
    normalRowMappings.add(new Array<int>(rowMappingsB));
}

// This creates new instances of the plugin...
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new mlrVSTAudioProcessor();
}
