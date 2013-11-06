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
#include "mlrVSTGUI.h"
#include "OSCHandler.h"
#include "GlobalSettings.h"
#include "Preset.h"
#include <cmath>

//==============================================================================
mlrVSTAudioProcessor::mlrVSTAudioProcessor() :
    // Global Settings
    gs(this),

    // MIDI / quantisation /////////////////////////////////////////
    quantisationOn(false), quantisationGap(0), quantRemaining(0),
    quantisedBuffer(), unquantisedCollector(),
    // Sample Pools ///////////////////////////
    samplePool(), resamplePool(), recordPool(),
    // Channel Setup ////////////////////////////////////////////////
    isMstrVolInc(false), isMstrVolDec(false),
    channelColours(),
    // Global Settings //////////////////////////////////////////////
    isBPMInc(false), isBPMDec(false),

    // Sample Strips //////////////////////
    sampleStripArray(),
    // OSC /////////////////////////////////////////////
    oscMsgHandler(gs.OSCPrefix, this),
    // Audio / MIDI Buffers /////////////////////////////////////////
    stripContrib(2, 0),
    resampleBuffer(2, 0), resampleLengthInSamples(0), resamplePrecountLengthInSamples(0),
    resamplePosition(0), resamplePrecountPosition(0),
    recordBuffer(2, 0), recordLengthInSamples(0), recordPrecountLengthInSamples(0),
    recordPosition(0), recordPrecountPosition(0),
    patternRecorder(),
    // Preset handling /////////////////////////////////////////
    presetList("preset_list"), setlist("setlist"),
    // Mapping settings ////////////////////////////////////////
    mappingEngine(), currentStripModifier(-1),
    // Misc /////////////////////////////////////////////////////////
    buttonStatus(gs.numMonomeRows, gs.numMonomeCols, false),
    playbackLEDPosition()
{

    // compile time assertions
    static_jassert(THUMBNAIL_WIDTH % 8 == 0);

    // start listening for messages
    oscMsgHandler.startThread();
    DBG("OSC thread started");

    // create our SampleStrip objects
    buildSampleStripArray(gs.numSampleStrips);

    // by updating the number of channels we build the mute / gain arrays
    gs.setGlobalSetting(GlobalSettings::sNumChannels, &(gs.maxChannels), false);



    // TODO: have actual sample rate
    for (int i = 0; i < gs.resampleBankSize; ++i)
        resamplePool.add(new AudioSample(44100.0, 44100, THUMBNAIL_WIDTH, "resample #" + String(i), AudioSample::tResampledSample));
    for (int i = 0; i < gs.recordBankSize; ++i)
        recordPool.add(new AudioSample(44100.0, 44100, THUMBNAIL_WIDTH, "record #" + String(i), AudioSample::tRecordedSample));
    for (int i = 0; i < gs.patternBankSize; ++i)
        patternRecordings.add(new PatternRecording(this, i));

    setlist.createNewChildElement("blank_preset");

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
    for (int s = 0; s < gs.numSampleStrips; s++)
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
    patternRecordings.clear(true);

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

    // use this to check that we are only loading audio files
    WildcardFileFilter fileFilter(getWildcardFormats(), " ", "audio files");
    // can straight up reject the file if it doesn't have the right extension
    if (!fileFilter.isFileSuitable(sampleFile))
    {
        DBG("Invalid file extension \"" << sampleFile.getFileExtension() << "\", loading aborted");
        return -1;
    }

    // otherwise try to load the Sample
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

void mlrVSTAudioProcessor::buildChannelArray()
{
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
    for (int c = 0; c < gs.numChannels; c++)
    {
        Colour channelColour(c / (float) (gs.numChannels), 0.5f, 0.5f, 1.0f);
        channelColours.add(channelColour);
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

        if (gs.useExternalTempo && getPlayHead() != 0 &&
            getPlayHead()->getCurrentPosition(newTime))
        {
            // Successfully got the current time from the host..
            lastPosInfo = newTime;
            if (gs.currentBPM != lastPosInfo.bpm && lastPosInfo.bpm > 1.0)
            {
				const double newBPM = lastPosInfo.bpm;

	            // If the tempo has changed, adjust the playspeeds accordingly
                gs.setGlobalSetting(GlobalSettings::sCurrentBPM, &newBPM);
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


        // use the pattern recorders (if recording)
        for (int b = 0; b < gs.patternBankSize; ++b)
            patternRecordings[b]->recordPattern(midiMessages, numSamples);

        // and play back patterns (if playing back)
        for (int b = 0; b < gs.patternBankSize; ++b)
            patternRecordings[b]->playPattern(midiMessages, numSamples);


        // if we are recording from mlrVST's inputs
        if (gs.isRecording)
            processRecordingBuffer(buffer, numSamples);


        // if we're aren't monitoring, clear any incoming audio
        if (!gs.monitorInputs) buffer.clear();


        // make sure the buffer for SampleStrip contributions is
        // *exactly* the right size and avoid reallocating if possible
        stripContrib.setSize(2, numSamples, false, false, true);

        for (int s = 0; s < sampleStripArray.size(); s++)
        {
            sampleStripArray[s]->setBPM(gs.currentBPM);

            // clear the contribution from the previous strip
            stripContrib.clear();
            // find this channels contribution
            sampleStripArray[s]->renderNextBlock(stripContrib, midiMessages, 0, numSamples);

            // get the associated channel so we can apply gain
            const int stripChannel = *static_cast<const int *>(sampleStripArray[s]->getSampleStripParam(SampleStrip::pCurrentChannel));

            // if this channel is NOT muted
            if (!getChannelMuteStatus(stripChannel))
            {
                // add this contribution scaled by the channel gain
                buffer.addFrom(0, 0, stripContrib, 0, 0, numSamples, getChannelGain(stripChannel) );
                buffer.addFrom(1, 0, stripContrib, 1, 0, numSamples, getChannelGain(stripChannel) );
            }
        }

        // Go through the outgoing data, and apply our master gain to it...
        for (int channel = 0; channel < getNumInputChannels(); ++channel)
            buffer.applyGain(channel, 0, buffer.getNumSamples(), gs.masterGain);


        // In case we have more outputs than inputs, we'll clear any output
        // channels that didn't contain input data, (because these aren't
        // guaranteed to be empty - they may contain garbage).
        for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        // if we are resampling the audio that mlrVST is producing...
        if (gs.isResampling)
            processResamplingBuffer(buffer, numSamples);

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

    /////////////////
    // Global updates

    // if either of the modifier buttons are lifted, stop increasing
    if (currentStripModifier != MappingEngine::rmGlobalMapping)
        isBPMInc = isBPMDec = isMstrVolInc = isMstrVolDec = false;

    if (isBPMInc)
    {
        const double newBPM = gs.currentBPM + 0.1;
        gs.setGlobalSetting(GlobalSettings::sCurrentBPM, &newBPM, true);
    }
    else if (isBPMDec)
    {
        const double newBPM = gs.currentBPM - 0.1;
        gs.setGlobalSetting(GlobalSettings::sCurrentBPM, &newBPM, true);
    }


    if (isMstrVolInc)
    {
        float newMasterGain = gs.masterGain + 0.05f;

        if (newMasterGain > 1.0f)
        {
            newMasterGain = 1.0f;
            isMstrVolInc = false;
        }

        gs.setGlobalSetting(GlobalSettings::sMasterGain, &newMasterGain, true);
    }
    else if (isMstrVolDec)
    {
        float newMasterGain = gs.masterGain - 0.05f;

        if (newMasterGain < 0.0f)
        {
            newMasterGain = 0.0f;
            isMstrVolDec = false;
        }

        gs.setGlobalSetting(GlobalSettings::sMasterGain, &newMasterGain, true);
    }


    //////////////////////
    // SampleStrip updates
    for (int row = 0; row < sampleStripArray.size(); ++row)
    {
        // first if either of the modifier buttons are lifted...
        if (currentStripModifier != MappingEngine::rmSampleStripMappingA
            && currentStripModifier != MappingEngine::rmSampleStripMappingB)
        {
            const bool stopIncsDecs = false;

            // ...stop any vol/speed incs/decs
            setSampleStripParameter(SampleStrip::pIsVolInc, &stopIncsDecs, row, false);
            setSampleStripParameter(SampleStrip::pIsVolDec, &stopIncsDecs, row, false);
            setSampleStripParameter(SampleStrip::pIsPlaySpeedInc, &stopIncsDecs, row, false);
            setSampleStripParameter(SampleStrip::pIsPlaySpeedDec, &stopIncsDecs, row, false);
        }



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
            if (stripVol > 4.0f)
            {
                stripVol = 4.0f;
                toggleSampleStripParameter(SampleStrip::pIsVolInc, row);
            }

            setSampleStripParameter(SampleStrip::pStripVolume, &stripVol, row);

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

        }

        else if (isSpeedDec)
        {
            double stripPlaySpeed = *static_cast<const double*>
                (getSampleStripParameter(SampleStrip::pPlaySpeed, row));
            stripPlaySpeed -= 0.01;
            setSampleStripParameter(SampleStrip::pPlaySpeed, &stripPlaySpeed, row);

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
    // filter out any keypresses outside the allowed range of the device
    if (monomeRow < 0 || monomeCol < 0) return;
    if (monomeRow >= gs.numMonomeRows || monomeCol >= gs.numMonomeCols) return;

    // globally track presses
    buttonStatus.set(monomeRow, monomeCol, state);
    sendChangeMessage();

    // if the button is on the top row
    if (monomeRow == 0)
    {
        // find out the mapping associated with it
        const int mappingID = getMonomeMapping(MappingEngine::rmTopRowMapping, monomeCol);

        // ...and act accordingly
        switch (mappingID)
        {
        case MappingEngine::tmNoMapping : break;
        case MappingEngine::tmSampleStripBtnA :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? MappingEngine::rmSampleStripMappingA : MappingEngine::rmNoBtn;
                break;
            }
        case MappingEngine::tmSampleStripBtnB :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? MappingEngine::rmSampleStripMappingB : MappingEngine::rmNoBtn;
                break;
            }
        case MappingEngine::tmPatternStripBtn :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? MappingEngine::rmPatternStripMapping : MappingEngine::rmNoBtn;
                break;
            }
        case MappingEngine::tmGlobalMappingBtn :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? MappingEngine::rmGlobalMapping : MappingEngine::rmNoBtn;
                break;
            }
        case MappingEngine::tmStartRecording : startRecording(); break;
        case MappingEngine::tmStartResampling : startResampling(); break;
        case MappingEngine::tmStopAll : stopAllStrips(SampleStrip::mStopNormal); break;
        case MappingEngine::tmTapeStopAll : stopAllStrips(SampleStrip::mStopTape); break;
        default : jassertfalse;
        }

        // we are done now
        return;
    }


    /* The -1 is because we are treating the second row on the device as
    the first "effective" row, the top row being reserved for other
    functions. Yes this is confusing.
    */
    const int stripID = monomeRow - 1;

    // check that we have enough SampleStrips...
    if (stripID >= gs.numSampleStrips) return;

    // if one of the normal row modifier buttons are held,
    // each strip now turns into a set of control buttons
    if ( (currentStripModifier == MappingEngine::rmSampleStripMappingA
       || currentStripModifier == MappingEngine::rmSampleStripMappingB) )
    {
        // first, find out which mapping is associated with the button
        const int mappingID = getMonomeMapping(currentStripModifier, monomeCol);

        // then excute that mapping
        executeSampleStripMapping(mappingID, stripID, state);

        // The SampleStrips can get tricked into thinking a button is held
        // if it starts out as a normal press but then a modifier button is
        // pressed before the button is lifted. This corrects for this:
        if (!state) sampleStripArray[stripID]->setButtonStatus(monomeCol, state);

    }


    // if PatternStrip modifier button is held, each strip now turns into
    // a set of control buttons that control that pattern recorder
    else if ( currentStripModifier == MappingEngine::rmPatternStripMapping )
    {
        // first, find out which mapping is associated with the button
        const int mappingID = getMonomeMapping(MappingEngine::rmPatternStripMapping, monomeCol);

        // then excute that mapping
        executePatternStripMapping(mappingID, stripID, state);

        // The SampleStrips can get tricked into thinking a button is held
        // if it starts out as a normal press but then a modifier button is
        // pressed before the button is lifted. This corrects for this:
        if (!state) sampleStripArray[stripID]->setButtonStatus(monomeCol, state);

    }

    // if GlobalMapping modifier button is held, the monome now turns into
    // a set of control columns that control the global mappings (tempo etc)
    else if ( currentStripModifier == MappingEngine::rmGlobalMapping )
    {
        // first, find out which mapping is associated with the button
        const int mappingID = getMonomeMapping(MappingEngine::rmGlobalMapping, monomeCol);

        // then excute that mapping (note doesn't depend on row,
        // meaning we can use any row to trigger it)
        executeGlobalMapping(mappingID, state);

        // The SampleStrips can get tricked into thinking a button is held
        // if it starts out as a normal press but then a modifier button is
        // pressed before the button is lifted. This corrects for this:
        if (!state) sampleStripArray[stripID]->setButtonStatus(monomeCol, state);

    }

    else
    {
        /* Only pass on messages that are from the allowed range of columns.
           NOTE: MIDI messages may still be passed from other sources that
           are outside this range so the channelProcessor must be aware of
           numChunks too to filter these.
        */

        const int numChunks = *static_cast<const int*>
            (sampleStripArray[stripID]->getSampleStripParam(SampleStrip::pNumChunks));

        // ditch anything outside this range
        if (monomeCol >= numChunks) return;


        // button pressed down
        if (state)
        {
            if (quantisationOn)
            {
                // If we are quantising, create an on message and store it it the queue -
                // then once the quantised interval (of length quantisationGap) has
                // elapsed (i.e. quantisation remaining < 0) we can fire in all the messages
                // at once!

                // NOTE: The +1 here is because midi channels start at 1 not 0!
                MidiMessage quantisedMessage(MidiMessage::noteOn(stripID + 1, monomeCol, 1.0f), 0.01);
                quantisedBuffer.addEvent(quantisedMessage, 0);
            }

            else
            {
                // NOTE: The +1 here is because midi channels start at 1 not 0!
                MidiMessage unquantisedMessage(MidiMessage::noteOn(stripID + 1, monomeCol, 1.0f), 0.01);

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
                // NOTE: The +1 here is because midi channels start at 1 not 0!
                MidiMessage quantisedMessage(MidiMessage::noteOff(stripID + 1, monomeCol), 0.01);
                quantisedBuffer.addEvent(quantisedMessage, 0);
            }
            else
            {
                // NOTE: The +1 here is because midi channels start at 1 not 0!
                MidiMessage unquantisedMessage(MidiMessage::noteOff(stripID + 1, monomeCol), 0.01);

                const double stamp = juce::Time::getMillisecondCounterHiRes() / 1000.0;
                unquantisedMessage.setTimeStamp(stamp);
                unquantisedCollector.addMessageToQueue(unquantisedMessage);
            }
        }
    }
}


///////////////////////
// SampleStrip stuff //
///////////////////////
void mlrVSTAudioProcessor::buildSampleStripArray(const int &newNumSampleStrips)
{
    jassert(newNumSampleStrips > 0);

    // make sure we're not using the sampleStripArray while (re)building it
    suspendProcessing(true);

    // if we are removing rows
    if (newNumSampleStrips < sampleStripArray.size())
    {
        const int numToRemove = sampleStripArray.size() - newNumSampleStrips;

        for (int i = 0; i < numToRemove; ++i)
            sampleStripArray.removeLast();
    }

    // otherwise if we are adding rows
    else if (newNumSampleStrips > sampleStripArray.size())
    {
        const int numToAdd = newNumSampleStrips - sampleStripArray.size();
        const int previousNumSampleStrips = sampleStripArray.size();

        for (int i = 0; i < numToAdd; ++i)
            sampleStripArray.add(new SampleStrip(previousNumSampleStrips + i, this));
    }


    DBG("Created " << newNumSampleStrips << " SampleStrips.");

    // resume processing
    suspendProcessing(false);
}
const double mlrVSTAudioProcessor::calcInitialPlaySpeed(const int &stripID, const bool &applyChange)
{
    // TODO insert proper host speed here
    return sampleStripArray[stripID]->findInitialPlaySpeed(gs.currentBPM, 44100.0, applyChange);
}
void mlrVSTAudioProcessor::calcPlaySpeedForNewBPM(const int &stripID)
{
    sampleStripArray[stripID]->updatePlaySpeedForBPMChange(gs.currentBPM);
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
    return sampleStripArray[index];
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

bool mlrVSTAudioProcessor::saveXmlSetlist(const File &setlistFile)
{
    DBG("Saving setlist to: " << setlistFile.getFullPathName());
    return setlist.writeToFile(setlistFile, String::empty);
}
bool mlrVSTAudioProcessor::loadXmlSetlist(const File &setlistFile)
{
    DBG("Loading setlist: " << setlistFile.getFullPathName());

    XmlDocument setlistToLoad(setlistFile);
    return Preset::loadSetlist(setlistToLoad.getDocumentElement(), this, &gs);
}

void mlrVSTAudioProcessor::createNewPreset(const String &newPresetName)
{
    // first create the preset
    XmlElement newPreset = Preset::createPreset(newPresetName, this, &gs);

    // see if there is an existing preset and replace it if so
    const bool replaceExisting = true;
    addPreset(&newPreset, replaceExisting);

    DBG(presetList.createDocument(String::empty));
}
void mlrVSTAudioProcessor::addPreset(XmlElement *newPreset, bool replaceExisting)
{
    // find the identifier for the preset attribute "name"
    const String nameAttribute = GlobalSettings::getGlobalSettingName(GlobalSettings::sPresetName);

    // get the new preset's name
    const String newPresetName = newPreset->getStringAttribute(nameAttribute);

    // and see if a preset of this name already exists in the preset list
    forEachXmlChildElement(presetList, p)
    {
        const String existingPresetName = p->getStringAttribute(nameAttribute);

        // if it does, replace it
        if (existingPresetName == newPresetName)
        {
            if (replaceExisting)
            {
                DBG("Replacing preset: '" << existingPresetName << "'.");
                presetList.replaceChildElement(p, new XmlElement(*newPreset));
            }
        }
    }

    // if we haven't found a match by here, the preset is unique
    // so add it as a new element
    presetList.addChildElement(new XmlElement(*newPreset));
}


void mlrVSTAudioProcessor::renamePreset(const String &newName, const int & presetID)
{
    const int presetListLength = presetList.getNumChildElements();

    if (presetID >= 0 && presetID < presetListLength)
    {
        XmlElement * elementToRename = presetList.getChildElement(presetID);
        elementToRename->setAttribute(GlobalSettings::getGlobalSettingName(GlobalSettings::sPresetName), newName);
    }
}
void mlrVSTAudioProcessor::removeSetlistItem(const int &id)
{
    const int setlistLength = setlist.getNumChildElements();
    if (id >= 0 && id < setlistLength)
    {
        XmlElement * elementToRemove = setlist.getChildElement(id);
        setlist.removeChildElement(elementToRemove, true);
    }
}
void mlrVSTAudioProcessor::removePresetListItem(const int &id)
{
    const int presetListLength = presetList.getNumChildElements();
    if (id >= 0 && id < presetListLength)
    {
        XmlElement * elementToRemove = presetList.getChildElement(id);
        presetList.removeChildElement(elementToRemove, true);
    }
}


void mlrVSTAudioProcessor::selectSetlistItem(const int &id)
{
    // get the preset at the specified index in the setlist
    XmlElement* setlistItemToLoad = setlist.getChildElement(id);
    if (setlistItemToLoad != nullptr) Preset::loadPreset(setlistItemToLoad, this, &gs);
}
void mlrVSTAudioProcessor::selectPresetListItem(const int &id)
{
    // get the preset at the specified index in the list of presets
    XmlElement* presetItemToLoad = presetList.getChildElement(id);
    if (presetItemToLoad != nullptr) Preset::loadPreset(presetItemToLoad, this, &gs);
}

void mlrVSTAudioProcessor::insetPresetIntoSetlist(const int &presetID, const int &indexToInsertAt)
{
    const int presetListLength = presetList.getNumChildElements();

    if (presetID >= 0 && presetID < presetListLength)
    {
        setlist.insertChildElement(new XmlElement(*(presetList.getChildElement(presetID))), indexToInsertAt);
    }
}



////////////////////////////
// RECORDING / RESAMPLING //
////////////////////////////

void mlrVSTAudioProcessor::startResampling()
{
    resampleLengthInSamples = (int) (getSampleRate() * (60.0 * gs.resampleLength / gs.currentBPM));
    resamplePrecountLengthInSamples = (int) (getSampleRate() * (60.0 * gs.resamplePrecountLength / gs.currentBPM));
    resamplePrecountPosition = resamplePrecountLengthInSamples;

    // TODO: try writing directly into the bank
    //resamplePool[resampleBank]->
    //resampleBuffer = resamplePool[resampleBank]->getAudioData();

    resampleBuffer.setSize(2, resampleLengthInSamples, false, true);
    resampleBuffer.clear();
    resamplePosition = 0;
    gs.isResampling = true;

    DBG("resampling started");
}
float mlrVSTAudioProcessor::getResamplingPrecountPercent() const
{
    if (resamplePrecountPosition <= 0 || resamplePrecountLengthInSamples <= 0)
        return 0.0;
    else
        return (float) (resamplePrecountPosition) / (float) (resamplePrecountLengthInSamples);
}
float mlrVSTAudioProcessor::getResamplingPercent() const
{
    if (resamplePosition >= resampleLengthInSamples || resampleLengthInSamples <= 0)
        return 0.0;
    else
        return (float) (resamplePosition) / (float) (resampleLengthInSamples);
}
void mlrVSTAudioProcessor::processResamplingBuffer(AudioSampleBuffer &buffer, const int &numSamples)
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
            gs.isResampling = false;

            // get the pointer to the slot in the resample bank that we want to replace
            AudioSampleBuffer *resampleSlotToReplace = resamplePool[gs.resampleBank]->getAudioData();
            // and resize it in case the length has changed
            resampleSlotToReplace->setSize(2, resampleLengthInSamples, false, false, false);
            // copy in the newly recorded samples
            resampleSlotToReplace->copyFrom(0, 0, resampleBuffer, 0, 0, resampleLengthInSamples);
            resampleSlotToReplace->copyFrom(1, 0, resampleBuffer, 1, 0, resampleLengthInSamples);
            // and update the thumbnail
            resamplePool[gs.resampleBank]->generateThumbnail(THUMBNAIL_WIDTH);

            // this is just so buttons can work out that it's finished
            resamplePosition += samplesToEnd;

            DBG("resample slot " << gs.resampleBank << " updated.");

            // make sure all SampleStrips redraw to reflect new waveform
            for (int s = 0; s < sampleStripArray.size(); ++s)
                sampleStripArray[s]->sendChangeMessage();
        }
    }
}

void mlrVSTAudioProcessor::startRecording()
{
    recordLengthInSamples = (int) (getSampleRate() * (60.0 * gs.recordLength / gs.currentBPM));
    recordPrecountLengthInSamples = (int) (getSampleRate() * (60.0 * gs.recordPrecountLength / gs.currentBPM));
    recordPrecountPosition = recordPrecountLengthInSamples;

    recordBuffer.setSize(2, recordLengthInSamples, false, true);
    recordBuffer.clear();
    recordPosition = 0;
    gs.isRecording = true;

    DBG("recording started");
}
float mlrVSTAudioProcessor::getRecordingPrecountPercent() const
{
    if (recordPrecountPosition <= 0 || recordPrecountLengthInSamples <= 0)
        return 0.0;
    else
        return (float) (recordPrecountPosition) / (float) (recordPrecountLengthInSamples);
}
float mlrVSTAudioProcessor::getRecordingPercent() const
{
    if (recordPosition >= recordLengthInSamples || recordLengthInSamples <= 0)
        return 0.0;
    else
        return (float) (recordPosition) / (float) (recordLengthInSamples);
}
void mlrVSTAudioProcessor::processRecordingBuffer(AudioSampleBuffer &buffer, const int &numSamples)
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
            gs.isRecording = false;

            // get the pointer to the slot in the record bank that we want to replace
            AudioSampleBuffer *recordSlotToReplace = recordPool[gs.recordBank]->getAudioData();
            // and resize it in case the length has changed
            recordSlotToReplace->setSize(2, recordLengthInSamples, false, false, false);
            // copy in the newly recorded samples
            recordSlotToReplace->copyFrom(0, 0, recordBuffer, 0, 0, recordLengthInSamples);
            recordSlotToReplace->copyFrom(1, 0, recordBuffer, 1, 0, recordLengthInSamples);
            // and update the thumbnail
            recordPool[gs.recordBank]->generateThumbnail(THUMBNAIL_WIDTH);

            DBG("record slot " << gs.recordBank << " updated.");

            // this is just so the TimedButtons can work out that recording has finished
            recordPosition += samplesToEnd;

            // make sure all SampleStrips redraw to reflect new waveform
            for (int s = 0; s < gs.numSampleStrips; ++s)
                sampleStripArray[s]->sendChangeMessage();
        }
    }
}


/////////////////////////////
// GETTERS AND SETTERS etc //
/////////////////////////////
AudioProcessorEditor* mlrVSTAudioProcessor::createEditor()
{
    return new mlrVSTGUI(this, gs.numChannels, gs.numSampleStrips);
}

void mlrVSTAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // This method stores parameters in the memory block

    // Create an outer XML element..
    XmlElement xml("GLOBALSETTINGS");

    //// add some attributes to it..

    //xml.setAttribute("MASTER_GAIN", gs.masterGain);
    //for (int c = 0; c < channelGains.size(); c++)
    //{
    //    String name("CHANNEL_GAIN");
    //    name += String(c);
    //    xml.setAttribute(name, channelGains[c]);
    //}

    //xml.setAttribute("CURRENT_PRESET", "none");

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary(xml, destData);
}
void mlrVSTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // TODO: this whole thing needs redone
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    //ScopedPointer<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    //if (xmlState != 0)
    //{
    //    // make sure that it's actually our type of XML object..
    //    if (xmlState->hasTagName("GLOBALSETTINGS"))
    //    {
    //        // ok, now pull out our parameters...
    //        gs.masterGain = (float) xmlState->getDoubleAttribute("MASTER_GAIN", defaultChannelGain);
    //        for (int c = 0; c < channelGains.size(); c++)
    //        {
    //            String name("CHANNEL_GAIN");
    //            name += String(c);
    //            channelGains.set(c, (float) xmlState->getDoubleAttribute(name, channelGains[c]));
    //        }
    //    }
    //}
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
bool mlrVSTAudioProcessor::silenceInProducesSilenceOut() const
{
#if JucePlugin_SilenceInProducesSilenceOut
    return true;
#else
    return false;
#endif
}
double mlrVSTAudioProcessor::getTailLengthSeconds() const
{
return 0.0;
}



void mlrVSTAudioProcessor::executeSampleStripMapping(const int &mappingID, const int &stripID, const bool &state)
{
    switch (mappingID)
    {
    case MappingEngine::nmNoMapping : break;
    case MappingEngine::nmFindBestTempo :
        // TODO: next actual sample rate here!
        if (state) sampleStripArray[stripID]->findInitialPlaySpeed(gs.currentBPM, 44100.0);
        break;

    case MappingEngine::nmToggleReverse :
        {
            if (state) sampleStripArray[stripID]->toggleSampleStripParam(SampleStrip::pIsReversed);
            break;
        }

    case MappingEngine::nmCycleThruChannels :
        {
            if (state) sampleStripArray[stripID]->cycleChannels();
            break;
        }
    case MappingEngine::nmDecVolume:
        {
            const bool isVolDec = state;
            sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pIsVolDec, &isVolDec, false);
            break;
        }

    case MappingEngine::nmIncVolume:
        {
            const bool isVolInc = state;
            sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pIsVolInc, &isVolInc, false);
            break;
        }

    case MappingEngine::nmDecPlayspeed:
        {
            const bool isSpeedDec = state;
            sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pIsPlaySpeedDec, &isSpeedDec, false);
            break;
        }

    case MappingEngine::nmIncPlayspeed:
        {
            const bool isSpeedInc = state;
            sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pIsPlaySpeedInc, &isSpeedInc, false);
            break;
        }

    case MappingEngine::nmHalvePlayspeed:
        {
            if (state) sampleStripArray[stripID]->modPlaySpeed(0.5);
            break;
        }

    case MappingEngine::nmDoublePlayspeed:
        {
            if (state) sampleStripArray[stripID]->modPlaySpeed(2.0);
            break;
        }

    case MappingEngine::nmSetNormalPlayspeed:
        {
            const double newPlaySpeed = 1.0;
            if (state) sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pPlaySpeed, &newPlaySpeed, true);
            break;
        }

    case MappingEngine::nmStopPlayback:
        {
            if (state) sampleStripArray[stripID]->stopSamplePlaying();
            break;
        }

    case MappingEngine::nmStopPlaybackTape:
        {
            if (state) sampleStripArray[stripID]->stopSamplePlaying(1);
            break;
        }

    case MappingEngine::nmCycleThruRecordings:
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

    case MappingEngine::nmCycleThruResamplings:
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

    case MappingEngine::nmCycleThruFileSamples:
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
}
void mlrVSTAudioProcessor::executePatternStripMapping(const int &mappingID, const int &stripID, const bool & /*state*/)
{
    // the pattern that we are modifing is determined by the row
    const int patternID = stripID;

    switch (mappingID)
    {
    case MappingEngine::patmapNoMapping : break;
    case MappingEngine::patmapStartRecording : patternRecordings[patternID]->startPatternRecording(); break;
    case MappingEngine::patmapStopRecording : patternRecordings[patternID]->stopPatternRecording(); break;
    case MappingEngine::patmapStartPlaying : patternRecordings[patternID]->startPatternPlaying(); break;
    case MappingEngine::patmapStopPlaying : patternRecordings[patternID]->stopPatternPlaying(); break;
	case MappingEngine::patmapDecLength :
		{
			if (patternRecordings[patternID]->patternLength > 1) (patternRecordings[patternID]->patternLength)--;
			break;
		}

	case MappingEngine::patmapIncLength :
		{
			if (patternRecordings[patternID]->patternLength < 32) (patternRecordings[patternID]->patternLength)++;
			break;
		}
	default : jassertfalse;
	}
}
void mlrVSTAudioProcessor::executeGlobalMapping(const int &mappingID, const bool & state)
{

    switch (mappingID)
    {
    case MappingEngine::gmBPMInc :
        {
            if (state)
            {
                isBPMInc = true; isBPMDec = false;
            }
            else
                isBPMInc = false;

            break;
        }
    case MappingEngine::gmBPMDec :
        {
            if (state)
            {
                isBPMDec = true; isBPMInc = false;
            }
            else
                isBPMDec = false;

            break;
        }
    case MappingEngine::gmNextPreset : DBG("next preset"); break;
    case MappingEngine::gmPrevPreset : DBG("prev preset"); break;
    case MappingEngine::gmMasterVolInc :
        {
            if (state)
            {
                isMstrVolInc = true; isMstrVolDec = false;
            }
            else
                isMstrVolInc = false;

            break;
        }
    case MappingEngine::gmMasterVolDec :
        {
            if (state)
            {
                isMstrVolDec = true; isMstrVolInc = false;
            }
            else
                isMstrVolDec = false;

            break;
        }

    case MappingEngine::gmNoMapping :
    default : jassertfalse;
    }
}



// helper / forwarding functions
const void* mlrVSTAudioProcessor::getGlobalSetting(const int &settingID) const
{
    return gs.getGlobalSetting(settingID);
}
void mlrVSTAudioProcessor::setGlobalSetting(const int &settingID, const void * newValue, const bool &notifyListeners)
{
    gs.setGlobalSetting(settingID, newValue, notifyListeners);
}



// This creates new instances of the plugin...
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new mlrVSTAudioProcessor();
}