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
    maxChannels(8), channelGains(), channelMutes(),
    masterGain(0.8f), isMstrVolInc(false), isMstrVolDec(false),
    defaultChannelGain(0.8f), channelColours(),
    // Global Settings //////////////////////////////////////////////
    numChannels(maxChannels), useExternalTempo(false),
    currentBPM(120.0), isBPMInc(false), isBPMDec(false),
    rampLength(50), numSampleStrips(7),
    // Sample Strips //////////////////////
    sampleStripArray(),
    // OSC /////////////////////////////////////////////
    OSCPrefix("mlrvst"), oscMsgHandler(OSCPrefix, this),
    // Audio / MIDI Buffers /////////////////////////////////////////
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
    patternRecorder(), currentPatternLength(8),
    currentPatternPrecountLength(0), currentPatternBank(0),
    patternBankSize(8),
    // Preset handling /////////////////////////////////////////
    presetList("PRESETLIST"), setlist("SETLIST"),
    // Mapping settings ////////////////////////////////////////
    topRowMappings(), sampleStripMappings(),
    patternStripMappings(), globalMappings(),
    numModifierButtons(2), currentStripModifier(-1),
    // Misc /////////////////////////////////////////////////////////
    monomeSize(eightByEight), numMonomeRows(8), numMonomeCols(8),
    buttonStatus(numMonomeRows, numMonomeCols, false),
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
    for (int i = 0; i < patternBankSize; ++i)
        patternRecordings.add(new PatternRecording(this, i));

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
				const double newBPM = lastPosInfo.bpm;

	            // If the tempo has changed, adjust the playspeeds accordingly
				updateGlobalSetting(mlrVSTAudioProcessor::sCurrentBPM, &newBPM);
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
        for (int b = 0; b < patternBankSize; ++b)
            patternRecordings[b]->recordPattern(midiMessages, numSamples);

        // and play back patterns (if playing back)
        for (int b = 0; b < patternBankSize; ++b)
            patternRecordings[b]->playPattern(midiMessages, numSamples);


        // if we are recording from mlrVST's inputs
        if (isRecording)
            processRecordingBuffer(buffer, numSamples);


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

        // if we are resampling the audio that mlrVST is producing...
        if (isResampling)
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
    if (currentStripModifier != rmGlobalMappingBtn)
        isBPMInc = isBPMDec = isMstrVolInc = isMstrVolDec = false;

    if (isBPMInc)
    {
        const double newBPM = currentBPM + 0.1;
        updateGlobalSetting(sCurrentBPM, &newBPM, true);
    }
    else if (isBPMDec)
    {
        const double newBPM = currentBPM - 0.1;
        updateGlobalSetting(sCurrentBPM, &newBPM, true);
    }


    if (isMstrVolInc)
    {
        float newMasterGain = masterGain + 0.05f;

        if (newMasterGain > 1.0f)
        {
            newMasterGain = 1.0f;
            isMstrVolInc = false;
        }

        updateGlobalSetting(sMasterGain, &newMasterGain, true);
    }
    else if (isMstrVolDec)
    {
        float newMasterGain = masterGain - 0.05f;

        if (newMasterGain < 0.0f)
        {
            newMasterGain = 0.0f;
            isMstrVolDec = false;
        }

        updateGlobalSetting(sMasterGain, &newMasterGain, true);
    }


    //////////////////////
    // SampleStrip updates
    for (int row = 0; row < sampleStripArray.size(); ++row)
    {
        // first if either of the modifier buttons are lifted...
        if (currentStripModifier != rmNormalRowMappingBtnA
            && currentStripModifier != rmNormalRowMappingBtnB)
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
    if (monomeRow >= numMonomeRows || monomeCol >= numMonomeCols) return;

    // globally track presses
    buttonStatus.set(monomeRow, monomeCol, state);
    sendChangeMessage();

    // if the button is on the top row
    if (monomeRow == 0)
    {
        // find out the mapping associated with it
        const int mappingID = getMonomeMapping(rmTopRowMapping, monomeCol);

        // ...and act accordingly
        switch (mappingID)
        {
        case tmNoMapping : break;
        case tmModifierBtnA :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? rmNormalRowMappingBtnA : rmNoBtn;
                break;
            }
        case tmModifierBtnB :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? rmNormalRowMappingBtnB : rmNoBtn;
                break;
            }
        case tmPatternModifierBtn :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? rmPatternBtn : rmNoBtn;
                break;
            }
        case tmGlobalModifierBtn :
            {
                // if pressed down, set this as the current
                // modifier button, otherwise set it to off (-1)
                currentStripModifier = (state) ? rmGlobalMappingBtn : rmNoBtn;
                break;
            }
        case tmStartRecording : startRecording(); break;
        case tmStartResampling : startResampling(); break;
        case tmStopAll : stopAllStrips(SampleStrip::mStopNormal); break;
        case tmTapeStopAll : stopAllStrips(SampleStrip::mStopTape); break;
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
    if (stripID >= numSampleStrips) return;

    // if one of the normal row modifier buttons are held,
    // each strip now turns into a set of control buttons
    if ( (currentStripModifier == rmNormalRowMappingBtnA
       || currentStripModifier == rmNormalRowMappingBtnB) )
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
    else if ( currentStripModifier == rmPatternBtn )
    {
        // first, find out which mapping is associated with the button
        const int mappingID = getMonomeMapping(rmPatternBtn, monomeCol);

        // then excute that mapping
        executePatternStripMapping(mappingID, stripID, state);

        // The SampleStrips can get tricked into thinking a button is held
        // if it starts out as a normal press but then a modifier button is
        // pressed before the button is lifted. This corrects for this:
        if (!state) sampleStripArray[stripID]->setButtonStatus(monomeCol, state);

    }

    // if GlobalMapping modifier button is held, the monome now turns into
    // a set of control columns that control the global mappings (tempo etc)
    else if ( currentStripModifier == rmGlobalMappingBtn )
    {
        // first, find out which mapping is associated with the button
        const int mappingID = getMonomeMapping(rmGlobalMappingBtn, monomeCol);

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

    numSampleStrips = newNumSampleStrips;
    DBG("Created " << newNumSampleStrips << " SampleStrips.");

    // resume processing
    suspendProcessing(false);
}
const double mlrVSTAudioProcessor::calcInitialPlaySpeed(const int &stripID, const bool &applyChange)
{
    // TODO insert proper host speed here
    return sampleStripArray[stripID]->findInitialPlaySpeed(currentBPM, 44100.0, applyChange);
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

void mlrVSTAudioProcessor::saveXmlSetlist(const File &setlistFile)
{
    DBG("Saving setlist to: " << setlistFile.getFullPathName());

    XmlElement globalSettings("SETTINGS");
}
void mlrVSTAudioProcessor::loadXmlSetlist(const File &setlistFile)
{
    DBG(setlistFile.getFullPathName());
}


void mlrVSTAudioProcessor::addPreset(const String &newPresetName)
{
    // TODO: check name not blank

    // this is now the current preset
    updateGlobalSetting(sPresetName, &newPresetName);

    // Create an outer XML element..
    XmlElement newPreset("PRESET");

    for (int s = 0; s < NumGlobalSettings; ++s)
    {
        // skip if we don't save this setting with each preset
        if (writeGlobalSettingToPreset(s) != ScopePreset) continue;

        // find if the setting is a bool, int, double etc.
        const int settingType = getGlobalSettingType(s);
        // what description do we write into the xml for this setting
        const String settingName = getGlobalSettingName(s);
        // get the actual parameter value (we cast later)
        const void *p = getGlobalSetting(s);

        switch (settingType)
        {
        case SampleStrip::TypeBool :
            newPreset.setAttribute(settingName, (int)(*static_cast<const bool*>(p))); break;
        case SampleStrip::TypeInt :
            newPreset.setAttribute(settingName, *static_cast<const int*>(p)); break;
        case SampleStrip::TypeDouble :
            newPreset.setAttribute(settingName, *static_cast<const double*>(p)); break;
        case SampleStrip::TypeFloat :
            newPreset.setAttribute(settingName, (double)(*static_cast<const float*>(p))); break;
        case SampleStrip::TypeString :
            newPreset.setAttribute(settingName, (*static_cast<const String*>(p))); break;
        default : jassertfalse;
        }

    }

    // add the master and channel volumes
    newPreset.setAttribute("master_vol", masterGain);

    for (int c = 0; c < numChannels; ++c)
        newPreset.setAttribute("chan_" + String(c) + "_vol", channelGains[c]);



    // Store the SampleStrip specific settings
    for (int strip = 0; strip < sampleStripArray.size(); ++strip)
    {
        XmlElement *stripXml = new XmlElement("STRIP");
        stripXml->setAttribute("id", strip);

        // write all parameters to XML
        for (int param = 0; param < SampleStrip::TotalNumParams; ++param)
        {
            // check if we are supposed to save this parameter, if not, skip
            if (!SampleStrip::isParamSaved(param)) continue;

            // find if the parameter is a bool, int, double etc.
            const int paramType = sampleStripArray[strip]->getParameterType(param);
            // what description do we write into the xml for this parameter
            const String paramName = sampleStripArray[strip]->getParameterName(param);
            // get the actual parameter value (we cast later)
            const void *p = sampleStripArray[strip]->getSampleStripParam(param);

            switch (paramType)
            {
            case SampleStrip::TypeBool :
                stripXml->setAttribute(paramName, (int)(*static_cast<const bool*>(p))); break;
            case SampleStrip::TypeInt :
                stripXml->setAttribute(paramName, *static_cast<const int*>(p)); break;
            case SampleStrip::TypeDouble :
                stripXml->setAttribute(paramName, *static_cast<const double*>(p)); break;
            case SampleStrip::TypeFloat :
                stripXml->setAttribute(paramName, (double)(*static_cast<const float*>(p))); break;
            case SampleStrip::TypeAudioSample :
                {
                    const AudioSample * sample = static_cast<const AudioSample*>(p);
                    if (sample)
                    {
                        const String samplePath = sample->getSampleFile().getFullPathName();
                        stripXml->setAttribute(paramName, samplePath);
                    }
                    break;
                }
            default : jassertfalse;
            }
        }

        // add this strip to the preset
        newPreset.addChildElement(stripXml);
    }

    const String nameAttribute = getGlobalSettingName(sPresetName);
    bool duplicateFound = false;

    // See if a preset of this name exists in the preset list
    forEachXmlChildElement(presetList, p)
    {
        const String pName = p->getStringAttribute(nameAttribute);

        // If it does, replace it
        if (pName == newPresetName)
        {
            presetList.replaceChildElement(p, new XmlElement(newPreset));
            duplicateFound = true;
            DBG("Replacing preset: '" << pName << "'.");
            break;
        }
    }

    // If the name is unique, add the new preset
    if (!duplicateFound) presetList.addChildElement(new XmlElement(newPreset));

    DBG(presetList.createDocument(String::empty));
}

void mlrVSTAudioProcessor::renamePreset(const String &newName, const int & presetID)
{
    const int presetListLength = presetList.getNumChildElements();

    if (presetID >= 0 && presetID < presetListLength)
    {
        XmlElement * elementToRename = presetList.getChildElement(presetID);
        elementToRename->setAttribute(getGlobalSettingName(sPresetName), newName);
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
    if (setlistItemToLoad != nullptr) loadPreset(setlistItemToLoad);
}
void mlrVSTAudioProcessor::selectPresetListItem(const int &id)
{
    // get the preset at the specified index in the list of presets
    XmlElement* presetItemToLoad = presetList.getChildElement(id);
    if (presetItemToLoad != nullptr) loadPreset(presetItemToLoad);
}

void mlrVSTAudioProcessor::insetPresetIntoSetlist(const int &presetID, const int &indexToInsertAt)
{
    const int presetListLength = presetList.getNumChildElements();

    if (presetID >= 0 && presetID < presetListLength)
    {
        setlist.insertChildElement(new XmlElement(*(presetList.getChildElement(presetID))), indexToInsertAt);
    }
}

void mlrVSTAudioProcessor::loadPreset(XmlElement * presetToLoad)
{
    // this *should* exist, if not, return doing nothing
    if (presetToLoad == nullptr)
    {
        jassertfalse;
        return;
    }


    if (presetToLoad->getTagName() == "PRESETNONE")
    {
        // we have a blank preset: loadDefaultPreset();
        DBG("blank preset loaded");
    }
    else if (presetToLoad->getTagName() == "PRESET")
    {
        const String newPresetName = presetToLoad->getStringAttribute("preset_name", "NOT FOUND");
        // check we know the name
        DBG("preset \"" << newPresetName << "\" loaded.");

        // TODO: first load the global parameters

        for (int s = 0; s < NumGlobalSettings; ++s)
        {
            // skip if we don't save this setting with each preset
            if (writeGlobalSettingToPreset(s) != ScopePreset) continue;

            // find if the setting is a bool, int, double etc.
            const int settingType = getGlobalSettingType(s);
            // what description do we write into the xml for this setting
            const String settingName = getGlobalSettingName(s);

            switch (settingType)
            {
            case SampleStrip::TypeBool :
                {
                    const bool value = presetToLoad->getIntAttribute(settingName) != 0;
                    updateGlobalSetting(s, &value, false); break;
                }
            case SampleStrip::TypeInt :
                {
                    const int value = presetToLoad->getIntAttribute(settingName);
                    updateGlobalSetting(s, &value, false); break;
                }
            case SampleStrip::TypeDouble :
                {
                    const double value = presetToLoad->getDoubleAttribute(settingName);
                    updateGlobalSetting(s, &value, false); break;
                }
            case SampleStrip::TypeFloat :
                {
                    const float value = (float) presetToLoad->getDoubleAttribute(settingName);
                    updateGlobalSetting(s, &value, false); break;
                }
            case SampleStrip::TypeString :
                {
                    const String value = presetToLoad->getStringAttribute(settingName);
                    updateGlobalSetting(s, &value, false); break;
                }
            default : jassertfalse;
            }

        }


        const float newMasterGain = (float) presetToLoad->getDoubleAttribute("master_vol", defaultChannelGain);
        if (newMasterGain >= 0.0 && newMasterGain <= 1.0) masterGain = newMasterGain;

        for (int c = 0; c < numChannels; ++c)
        {
            const String chanVolName = "chan_" + String(c) + "_vol";
            const float chanGain = (float) presetToLoad->getDoubleAttribute(chanVolName, defaultChannelGain);
            channelGains.set(c, (chanGain >= 0.0 && chanGain <= 1.0) ? chanGain : defaultChannelGain);
        }



        // try to find settings for each strip *that currently exists*
        for (int s = 0; s < sampleStripArray.size(); ++s)
        {
            // search the preset for the strip with matching ID
            forEachXmlChildElement(*presetToLoad, strip)
            {
                const int stripID = strip->getIntAttribute("id", -1);
                // check if the preset has information for this SampleStrip
                if (stripID != s) continue;

                SampleStrip *currentStrip = sampleStripArray[stripID];

                // if so, try to extract all the required parameters
                for (int param = 0; param < SampleStrip::TotalNumParams; ++param)
                {
                    // check that this is a parameter that we load
                    const bool doWeLoadParam = SampleStrip::isParamSaved(param);
                    if (!doWeLoadParam) continue;

                    // what name is this parameter?
                    const String paramName = SampleStrip::getParameterName(param);

                    // check that it can be found in the XML
                    if (strip->hasAttribute(paramName))
                    {
                        // find its type
                        const int paramType = SampleStrip::getParameterType(param);

                        // DBG("id: " << stripID << " " << paramName << " " << param << " " << paramType);

                        // try to load it
                        switch (paramType)
                        {
                        case TypeInt :
                            {
                                // TODO: validate values here
                                const int value = strip->getIntAttribute(paramName);
                                currentStrip->setSampleStripParam(param, &value);
                                break;
                            }
                        case TypeFloat :
                            {
                                const float value = (float) strip->getDoubleAttribute(paramName);
                                currentStrip->setSampleStripParam(param, &value);
                                break;
                            }
                        case TypeDouble :
                            {
                                const double value = strip->getDoubleAttribute(paramName);
                                currentStrip->setSampleStripParam(param, &value);
                                break;
                            }
                        case TypeBool :
                            {
                                const bool value = strip->getBoolAttribute(paramName);
                                currentStrip->setSampleStripParam(param, &value);
                                break;
                            }
                        case TypeAudioSample :
                            {
                                const String filePath = strip->getStringAttribute(paramName);
                                File newFile = File(filePath);

                                // add the sample and get the id
                                const int sampleID = addNewSample(newFile);
                                const AudioSample * newSample = getAudioSample(sampleID, pSamplePool);

                                // if the sample loaded correctly, set it to be the strip's AudioSample
                                if (sampleID != -1 && newSample)
                                    currentStrip->setSampleStripParam(SampleStrip::pAudioSample, newSample);
                            }
                        } // end switch statement

                    } // end of if(hasAtttribute)
                    else
                    {
                        DBG("Param " << paramName << " not found. Doing nothing");
                        // TODO: load default
                        // OR do nothing
                    }

                } // end loop over expected parameters

                // we have loaded this strip's parameters so can stop searching
                break;

            } // end of loop over preset entries

        }// end of samplestrip loop

        // let the GUI know that we have reloaded
        sendChangeMessage();
    }
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


/////////////////////////////
// GETTERS AND SETTERS etc //
/////////////////////////////
AudioProcessorEditor* mlrVSTAudioProcessor::createEditor()
{
    return new mlrVSTGUI(this, numChannels, numSampleStrips);
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


// Global settings ///////////////////////
int mlrVSTAudioProcessor::writeGlobalSettingToPreset(const int &settingID)
{
    // Explanation of codes:
    //  - ScopeError: we went wrong somewhere!
    //  - ScopeNone: don't save/load this setting
    //  - ScopePreset: can vary with every preset
    //  - ScopeSetlist: saved once globally

    switch (settingID)
    {
    case sUseExternalTempo : return ScopeSetlist;
    case sNumChannels : return ScopePreset;
    case sMonomeSize : return ScopeSetlist;
    case sPresetName : return ScopePreset;
    case sNumMonomeRows : return 0;
    case sNumMonomeCols : return 0;
    case sNumSampleStrips : return ScopeSetlist;
    case sMasterGain : return ScopePreset;
    case sCurrentBPM : return ScopePreset;
    case sQuantiseLevel : return 0;
    case sQuantiseMenuSelection : return ScopePreset;
    case sOSCPrefix : return ScopeSetlist;
    case sMonitorInputs : return ScopePreset;
    case sRecordPrecount : return ScopePreset;
    case sRecordLength : return ScopePreset;
    case sRecordBank : return ScopePreset;
    case sResamplePrecount : return ScopePreset;
    case sResampleLength : return ScopePreset;
    case sResampleBank : return ScopePreset;
    case sPatternPrecount : return ScopePreset;
    case sPatternLength : return ScopePreset;
    case sPatternBank : return ScopePreset;
    case sRampLength : return ScopeSetlist;
    default : jassertfalse; return ScopeError;
    }
}
String mlrVSTAudioProcessor::getGlobalSettingName(const int &settingID)
{
    switch (settingID)
    {
    case sUseExternalTempo : return "use_external_tempo";
    case sPresetName : return "preset_name";
    case sNumChannels : return "num_channels";
    case sMonomeSize : return "monome_size";
    case sNumSampleStrips : return "num_sample_strips";
    case sMasterGain : return "master_gain";
    case sCurrentBPM : return "current_bpm";
    case sQuantiseLevel : return "quantisation_level";
    case sRampLength : return "ramp_length";
    case sQuantiseMenuSelection : return "quantize_level";
    case sMonitorInputs : return "monitor_inputs";
    case sRecordPrecount : return "record_length";
    case sRecordLength : return "record_length";
    case sRecordBank : return "record_bank";
    case sResamplePrecount : return "resample_precount";
    case sResampleLength : return "resample_length";
    case sResampleBank : return "resample_bank";
    case sPatternPrecount : return "pattern_precount";
    case sPatternLength : return "pattern_length";
    case sPatternBank : return "pattern_bank";
    default : jassertfalse; return "name_not_found";
    }
}
int mlrVSTAudioProcessor::getGlobalSettingID(const String &settingName)
{
    // TODO: is this needed?
    if (settingName == "num_channels") return sNumChannels;
    if (settingName == "use_external_tempo") return sUseExternalTempo;
    if (settingName == "num_channels") return sNumChannels;
    if (settingName == "monome_size") return sMonomeSize;
    if (settingName == "num_sample_strips") return sNumSampleStrips;
    if (settingName == "current_bpm") return sCurrentBPM;
    if (settingName == "quantisation_level") return sQuantiseLevel;
    if (settingName == "ramp_length") return sRampLength;

    jassertfalse; return -1;

}
int mlrVSTAudioProcessor::getGlobalSettingType(const int &settingID)
{
    switch (settingID)
    {
    case sUseExternalTempo : return TypeBool;
    case sPresetName : return TypeString;
    case sNumChannels : return TypeInt;
    case sMonomeSize : return TypeInt;
    case sNumSampleStrips : return TypeInt;
    case sMasterGain : return TypeFloat;
    case sCurrentBPM : return TypeDouble;
    case sQuantiseLevel : return TypeDouble;
    case sRampLength : return TypeInt;
    case sQuantiseMenuSelection : return TypeInt;
    case sMonitorInputs : return TypeBool;
    case sRecordPrecount : return TypeInt;
    case sRecordLength : return TypeInt;
    case sRecordBank : return TypeInt;
    case sResamplePrecount : return TypeInt;
    case sResampleLength : return TypeInt;
    case sResampleBank : return TypeInt;
    case sPatternPrecount : return TypeInt;
    case sPatternLength : return TypeInt;
    case sPatternBank : return TypeInt;
    default : jassertfalse; return TypeError;
    }
}

void mlrVSTAudioProcessor::updateGlobalSetting(const int &settingID,
                                               const void *newValue,
                                               const bool &notifyListeners)
{
    switch (settingID)
    {
    case sUseExternalTempo :
        useExternalTempo = *static_cast<const bool*>(newValue); break;

    case sPresetName :
        presetName = *static_cast<const String*>(newValue); break;

    case sNumChannels :
		{
			numChannels = *static_cast<const int*>(newValue);
			buildChannelArray(numChannels);
			break;
		}

    case sMonomeSize :
        {
            monomeSize = *static_cast<const int*>(newValue);

            switch(monomeSize)
            {
            case eightByEight :     numMonomeRows = numMonomeCols = 8;
            case sixteenByEight :   numMonomeRows = 16; numMonomeCols = 8;
            case eightBySixteen :   numMonomeRows = 8; numMonomeCols = 16;
            case sixteenBySixteen : numMonomeRows = numMonomeCols = 16;
            default : jassertfalse;
            }

            buttonStatus.setSize(numMonomeRows, numMonomeCols, false);
            break;
        }

    case sNumSampleStrips :
        {
            const int newNumSampleStrips = *static_cast<const int*>(newValue);

            if (newNumSampleStrips != numSampleStrips)
            {
                buildSampleStripArray(newNumSampleStrips);
                numSampleStrips = newNumSampleStrips;
            }
            break;
        }

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

    case sPatternLength :
        currentPatternLength = *static_cast<const int*>(newValue);
        patternRecordings[currentPatternBank]->patternLength = currentPatternLength;
        break;

    case sPatternPrecount :
        currentPatternPrecountLength = *static_cast<const int*>(newValue);
        patternRecordings[currentPatternBank]->patternPrecountLength = currentPatternPrecountLength;
        break;

    case sPatternBank :
        currentPatternBank = *static_cast<const int*>(newValue);

        // and load the settings associated with that bank
        currentPatternLength = patternRecordings[currentPatternBank]->patternLength;
        currentPatternPrecountLength = patternRecordings[currentPatternBank]->patternPrecountLength;
        break;

    case sMasterGain :
        {
            masterGain = *static_cast<const float*>(newValue);
            break;
        }

    case sCurrentBPM :
        {
            currentBPM = *static_cast<const double*>(newValue);
            changeBPM();

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

    // if requested, let listeners know that a global setting has changed
    if (notifyListeners) sendChangeMessage();
}
const void* mlrVSTAudioProcessor::getGlobalSetting(const int &settingID) const
{
    switch (settingID)
    {
    case sUseExternalTempo : return &useExternalTempo;
    case sPresetName : return &presetName;
    case sNumChannels : return &numChannels;
    case sMonomeSize : return &monomeSize;
    case sNumMonomeRows : return &numMonomeRows;
    case sNumMonomeCols : return &numMonomeCols;
    case sNumSampleStrips : return &numSampleStrips;
    case sOSCPrefix : return &OSCPrefix;
    case sMonitorInputs : return &monitorInputs;
    case sMasterGain: return &masterGain;
    case sCurrentBPM : return &currentBPM;
    case sQuantiseLevel : return &quantisationLevel;
    case sQuantiseMenuSelection : return &quantiseMenuSelection;
    case sRecordLength : return &recordLength;
    case sRecordPrecount : return &recordPrecountLength;
    case sRecordBank : return &recordBank;
    case sResampleLength : return &resampleLength;
    case sResamplePrecount : return &resamplePrecountLength;
    case sResampleBank : return &resampleBank;

    case sPatternLength : return &currentPatternLength;
    case sPatternPrecount : return &currentPatternPrecountLength;
    case sPatternBank : return &currentPatternBank;
    case sRampLength : return &rampLength;
    default : jassertfalse; return 0;
    }
}



// Mappings /////////////////////

int mlrVSTAudioProcessor::getMonomeMapping(const int &rowType, const int &col) const
{
    switch (rowType)
    {
    case rmTopRowMapping : return topRowMappings[col];
    case rmNormalRowMappingBtnA : return sampleStripMappings[0]->getUnchecked(col);
    case rmNormalRowMappingBtnB : return sampleStripMappings[1]->getUnchecked(col);
    case rmPatternBtn : return patternStripMappings[col];
    case rmGlobalMappingBtn : return globalMappings[col];

    case rmNoBtn :
    default : jassertfalse; return -1;
    }
}
void mlrVSTAudioProcessor::setMonomeMapping(const int &rowType, const int &col, const int &newMapping)
{
    switch (rowType)
    {
    case rmTopRowMapping : topRowMappings.set(col, newMapping);
    case rmNormalRowMappingBtnA : sampleStripMappings[0]->set(col, newMapping);
    case rmNormalRowMappingBtnB : sampleStripMappings[1]->set(col, newMapping);
    case rmPatternBtn : return patternStripMappings.set(col, newMapping);
    case rmGlobalMappingBtn : return patternStripMappings.set(col, newMapping);

    case rmNoBtn :
    default : jassertfalse
    }
}

String mlrVSTAudioProcessor::getTopRowMappingName(const int &mappingID)
{
    switch (mappingID)
    {
    case tmNoMapping : return "no mapping";
    case tmModifierBtnA : return "modifier button A";
    case tmModifierBtnB : return "modifier button B";
    case tmPatternModifierBtn : return "pattern modifier button";
    case tmGlobalModifierBtn : return "global modifier btn";
    case tmStartRecording : return "start recording";
    case tmStartResampling : return "stop recording";
    case tmStopAll : return "stop all strips";
    case tmTapeStopAll : return "tape stop all strips";
    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";
    }
}
String mlrVSTAudioProcessor::getSampleStripMappingName(const int &mappingID)
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
    case nmSetNormalPlayspeed : return "set speed to 1.0";
    case nmStopPlayback : return "stop playback";
    case nmStopPlaybackTape : return "stop playback (tape)";
    case nmCycleThruRecordings : return "cycle through recordings";
    case nmCycleThruResamplings : return "cycle through resamples";
    case nmCycleThruFileSamples : return "cycle through samples";

    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";
    }
}
String mlrVSTAudioProcessor::getPatternStripMappingName(const int &mappingID)
{
    switch (mappingID)
    {
    case patmapNoMapping : return "no mapping";
    case patmapStartRecording : return "start recording";
    case patmapStopRecording : return "stop recording";
    case patmapStartPlaying : return "start playing";
    case patmapStopPlaying : return "stop playing";
	case patmapDecLength : return "decrease length";
	case patmapIncLength : return "increase length";

    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";

    }
}
String mlrVSTAudioProcessor::getGlobalMappingName(const int &mappingID)
{

    switch (mappingID)
    {

    case gmNoMapping : return "no mapping";
    case gmBPMInc : return "inc BPM";
    case gmBPMDec : return "dec BPM";
    case gmNextPreset : return "next preset";
    case gmPrevPreset : return "prev preset";
    case gmMasterVolInc : return "inc mstr vol";
    case gmMasterVolDec : return "dec mstr vol";

    default : jassertfalse; return "error: mappingID " + String(mappingID) + " not found!";

    }
}



void mlrVSTAudioProcessor::executeSampleStripMapping(const int &mappingID, const int &stripID, const bool &state)
{
    switch (mappingID)
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
}
void mlrVSTAudioProcessor::executePatternStripMapping(const int &mappingID, const int &stripID, const bool & /*state*/)
{
    // the pattern that we are modifing is determined by the row
    const int patternID = stripID;

    switch (mappingID)
    {
    case patmapNoMapping : break;
    case patmapStartRecording : patternRecordings[patternID]->startPatternRecording(); break;
    case patmapStopRecording : patternRecordings[patternID]->stopPatternRecording(); break;
    case patmapStartPlaying : patternRecordings[patternID]->startPatternPlaying(); break;
    case patmapStopPlaying : patternRecordings[patternID]->stopPatternPlaying(); break;
	case patmapDecLength :
		{
			if (patternRecordings[patternID]->patternLength > 1) (patternRecordings[patternID]->patternLength)--;
			break;
		}

	case patmapIncLength :
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
    case gmBPMInc :
        {
            if (state)
            {
                isBPMInc = true; isBPMDec = false;
            }
            else
                isBPMInc = false;

            break;
        }
    case gmBPMDec :
        {
            if (state)
            {
                isBPMDec = true; isBPMInc = false;
            }
            else
                isBPMDec = false;

            break;
        }
    case gmNextPreset : DBG("next preset"); break;
    case gmPrevPreset : DBG("prev preset"); break;
    case gmMasterVolInc :
        {
            if (state)
            {
                isMstrVolInc = true; isMstrVolDec = false;
            }
            else
                isMstrVolInc = false;

            break;
        }
    case gmMasterVolDec :
        {
            if (state)
            {
                isMstrVolDec = true; isMstrVolInc = false;
            }
            else
                isMstrVolDec = false;

            break;
        }

    case gmNoMapping :
    default : jassertfalse;
    }
}



void mlrVSTAudioProcessor::setupDefaultRowMappings()
{
    // clear any existing mappings
    topRowMappings.clear();
    sampleStripMappings.clear();
    patternStripMappings.clear();
    globalMappings.clear();

    // add the defaults
    topRowMappings.add(tmModifierBtnA);
    topRowMappings.add(tmModifierBtnB);
    topRowMappings.add(tmPatternModifierBtn);
    topRowMappings.add(tmGlobalModifierBtn);
    topRowMappings.add(tmStopAll);
    topRowMappings.add(tmTapeStopAll);
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

    sampleStripMappings.add(new Array<int>(rowMappingsA));
    sampleStripMappings.add(new Array<int>(rowMappingsB));

    patternStripMappings.add(patmapStartRecording);
    patternStripMappings.add(patmapStopRecording);
    patternStripMappings.add(patmapStartPlaying);
    patternStripMappings.add(patmapStopPlaying);
    patternStripMappings.add(patmapDecLength);
    patternStripMappings.add(patmapIncLength);
    patternStripMappings.add(patmapNoMapping);
    patternStripMappings.add(patmapNoMapping);

    globalMappings.add(gmNextPreset);
    globalMappings.add(gmPrevPreset);
    globalMappings.add(gmBPMDec);
    globalMappings.add(gmBPMInc);
    globalMappings.add(gmMasterVolDec);
    globalMappings.add(gmMasterVolInc);
    globalMappings.add(gmNoMapping);
    globalMappings.add(gmNoMapping);
}

// This creates new instances of the plugin...
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new mlrVSTAudioProcessor();
}