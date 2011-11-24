/*
==============================================================================

This file was auto-generated by the Jucer!

It contains the basic startup code for a Juce application.

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
    currentBPM(120.0), channelColours(),
    maxChannels(8), numChannels(maxChannels),
    sampleStripArray(), numSampleStrips(7),
    channelGains(), defaultChannelGain(0.8f),
    samplePool(),               // sample pool is initially empty
    oscMsgHandler(this),
    stripModifier(false),
    stripContrib(2, 0),
    resampleBuffer(2, 0), isResampling(false),
    recordBuffer(2, 0), isRecording(false),
    presetList("PRESETLIST"), setlist("SETLIST"),
    playbackLEDPosition(), buttonStatus()
{
    
    DBG("starting OSC thread");
    
    // start listening for messages
    oscMsgHandler.startThread();

    //File test("C:\\Users\\Hemmer\\Desktop\\funky.wav");
    //samplePool.add(new AudioSample(test));
    samplePool.clear();

    // Set up some default values..
    masterGain = 1.0f;

    // create our SampleStrip objects 
    buildSampleStripArray(numSampleStrips);
    // add our channels
    buildChannelArray(numChannels);

    setlist.createNewChildElement("PRESET_NONE");

    lastPosInfo.resetToDefault();

    startTimer(100);

    // setup 2D arrays for tracking button presses
    // or LED status
    setMonomeStatusGrids(8, 8);
}

mlrVSTAudioProcessor::~mlrVSTAudioProcessor()
{
    // be polite and turn off any remaining LEDs!
    oscMsgHandler.clearGrid();

    samplePool.clear(true);
    sampleStripArray.clear(true);

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
        samplePool.add(new AudioSample(sampleFile));
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

void mlrVSTAudioProcessor::setMonomeStatusGrids(const int &width, const int &height)
{
    // ignore the top row (reserved for other things)
    int effectiveHeight = height - 1;

    // reset arrays
    playbackLEDPosition.clear();
    buttonStatus.clear();

    for (int i = 0; i < effectiveHeight; ++i)
    {
        playbackLEDPosition.add(-1);
        Array<bool> temp;
        for (int j = 0; j < width; ++j)
        {
            temp.add(false);
        }
        
        buttonStatus.add(new Array<bool>(temp));
    }
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

    // clear MIDI queue
    monomeState.reset();
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
    for (int c = 0; c < maxChannels; c++)
        channelGains.add(defaultChannelGain);


    DBG("Channel processor array built");

    // resume processing
    suspendProcessing(false);
}


//==============================================================================
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

//==============================================================================
void mlrVSTAudioProcessor::prepareToPlay(double /*sampleRate*/, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // TODO: does ChannelProcessor need this?
    //synth.setCurrentPlaybackSampleRate (sampleRate);
    monomeState.reset();

    // this is not a completely accurate size as the block size may change with 
    // time, but at least we can allocate roughly the right size:
    stripContrib.setSize(2, samplesPerBlock, false, true, false);
}

void mlrVSTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    monomeState.reset();
}

void mlrVSTAudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers, etc, as it
    // means there's been a break in the audio's continuity.
}

void mlrVSTAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    if (!isSuspended())
    {
        // ask the host for the current time so we can display it...
        AudioPlayHead::CurrentPositionInfo newTime;

        if (useExternalTempo &&
            getPlayHead() != 0 &&
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


        const int numSamples = buffer.getNumSamples();

        /* this adds the OSC messages from the monome which have been converted
        to midi messages (where MIDI channel is row, note number is column) */
        monomeState.processNextMidiBuffer(midiMessages, 0, numSamples, true);




        if (isRecording)
        {
            if (numSamples + recordPosition < recordLength)
            {
                recordBuffer.addFrom(0, recordPosition, buffer, 0, 0, numSamples);
                recordBuffer.addFrom(1, recordPosition, buffer, 1, 0, numSamples);
                recordPosition += numSamples;
            }
            else
            {
                const int samplesToEnd = recordLength - recordPosition;
                recordBuffer.addFrom(0, recordPosition, buffer, 0, 0, samplesToEnd);
                recordBuffer.addFrom(1, recordPosition, buffer, 1, 0, samplesToEnd);
                isRecording = false;

                DBG("recording stopped");
                samplePool.add(new AudioSample(recordBuffer, getSampleRate()));
            }
        }

        if (!monitorInputs)
        {
            buffer.clear();
        }


        /* TODO: this actually isn't a very nice way to do this, eventually
           SampleStrips should do the processing then just add it to the
           relevant channel. This would make a whole load of things easier!
        */
        // for each channel, add its contributions
        // Remember to set the correct sample
        //for (int c = 0; c < channelProcessorArray.size(); c++)
        //{
        //    channelProcessorArray[c]->setBPM(currentBPM);
        //    channelProcessorArray[c]->getCurrentPlaybackPercentage();
        //    channelProcessorArray[c]->renderNextBlock(buffer, midiMessages, 0, numSamples);
        //}

        // make sure the buffer for SampleStrip contributions is
        // *exactly* the right size and avoid reallocating if possible
        stripContrib.setSize(2, numSamples, false, false, true);

        int stripChannel;
        for (int s = 0; s < sampleStripArray.size(); s++)
        {
            sampleStripArray[s]->setBPM(currentBPM);
            // TODO: might be better to return an entire buffer object then apply gain
            stripContrib.clear();
            sampleStripArray[s]->renderNextBlock(stripContrib, midiMessages, 0, numSamples);

            // get the associated channel so we can apply gain
            stripChannel = *static_cast<const int *>(sampleStripArray[s]->getSampleStripParam(SampleStrip::pCurrentChannel));
            
            buffer.addFrom(0, 0, stripContrib, 0, 0, numSamples, channelGains[stripChannel]);
            buffer.addFrom(1, 0, stripContrib, 1, 0, numSamples, channelGains[stripChannel]);
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
            if (numSamples + resamplePosition < resampleLength)
            {
                resampleBuffer.addFrom(0, resamplePosition, buffer, 0, 0, numSamples);
                resampleBuffer.addFrom(1, resamplePosition, buffer, 1, 0, numSamples);
                resamplePosition += numSamples;
            }
            else
            {
                const int samplesToEnd = resampleLength - resamplePosition;
                resampleBuffer.addFrom(0, resamplePosition, buffer, 0, 0, samplesToEnd);
                resampleBuffer.addFrom(1, resamplePosition, buffer, 1, 0, samplesToEnd);
                isResampling = false;
                DBG("resampling stopped");
                samplePool.add(new AudioSample(resampleBuffer, getSampleRate()));
            }
        }

        
    }
    else
    {
        //silence
    }
}

//==============================================================================
AudioProcessorEditor* mlrVSTAudioProcessor::createEditor()
{
    return new mlrVSTAudioProcessorEditor(this, numChannels);
}

//==============================================================================
void mlrVSTAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // This method stores parameters in the memory block

    // Create an outer XML element..
    XmlElement xml("GLOBALSETTINGS");

    // add some attributes to it..

    xml.setAttribute("master gain", masterGain);
    for (int c = 0; c < channelGains.size(); c++)
    {
        String name("channel gain ");
        name += c;
        xml.setAttribute(name, channelGains[c]);
    }
     
    xml.setAttribute("current preset", "none");

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
            masterGain  = (float) xmlState->getDoubleAttribute("master gain", masterGain);
            for (int c = 0; c < channelGains.size(); c++)
            {
                String name("channel gain ");
                name += c;
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

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new mlrVSTAudioProcessor();
}


//////////////////////
// OSC Stuff        //
//////////////////////

void mlrVSTAudioProcessor::timerCallback()
{


    
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
            if (!stripModifier) toggleSampleStripParameter(SampleStrip::pIsVolInc, row);
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
            if (!stripModifier) toggleSampleStripParameter(SampleStrip::pIsVolDec, row);
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
            if (!stripModifier) toggleSampleStripParameter(SampleStrip::pIsPlaySpeedInc, row);
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
            if (!stripModifier) toggleSampleStripParameter(SampleStrip::pIsPlaySpeedDec, row);
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
    const int effectiveMonomeRow = monomeRow - 1;



    /* When the top left button is held, each strip turns into a
       set of control buttons. See below for the mapping.
    */
    if (monomeRow == 0 && monomeCol == 0)
    {
        stripModifier = state;
    }


    if (monomeRow == 0)
    {
        /*
        // TODO proper control mappings for top row
        // stops channels 1-N playing
        if (monomeCol < numChannels)
        {
            for (int s = 0; s < sampleStripArray.size(); ++s)
            {
                const int chan = *static_cast<const int*>(sampleStripArray[s]->getSampleStripParam(SampleStrip::pCurrentChannel));
                if (chan == monomeCol)
                    sampleStripArray[s]->stopSamplePlaying();
            }
        }
        */
    }
    else if (stripModifier && monomeRow <= numSampleStrips && monomeRow > 0)
    {
        switch (monomeCol)
        {
        case 0 :
            if (state)
                sampleStripArray[effectiveMonomeRow]->stopSamplePlaying();
            break;

        case 1:
            if (state)
                sampleStripArray[effectiveMonomeRow]->toggleSampleStripParam(SampleStrip::pIsReversed);
            break;

        case 2:
            {
                const bool isVolDec = state;
                sampleStripArray[effectiveMonomeRow]->setSampleStripParam(SampleStrip::pIsVolDec, &isVolDec, false);
                break;
            }

        case 3:
            {
                const bool isVolInc = state;
                sampleStripArray[effectiveMonomeRow]->setSampleStripParam(SampleStrip::pIsVolInc, &isVolInc, false);
                break;
            }

        case 4:
            {
                const bool isSpeedDec = state;
                sampleStripArray[effectiveMonomeRow]->setSampleStripParam(SampleStrip::pIsPlaySpeedDec, &isSpeedDec, false);
                break;
            }

        case 5:
            {
                const bool isSpeedInc = state;
                sampleStripArray[effectiveMonomeRow]->setSampleStripParam(SampleStrip::pIsPlaySpeedInc, &isSpeedInc, false);
                break;
            }


        }
    }
    else if (monomeRow <= numSampleStrips && monomeRow > 0)
    {

        /* Only pass on messages that are from the allowed range of columns.
           NOTE: MIDI messages may still be passed from other sources that
           are outside this range so the channelProcessor must be aware of 
           numChunks too to filter these. 


        /* Button presses are tracked in a boolean array for each row to allow 
           for special combos for stopping clips etc. For this we need to know
           which button in the row is furthest left.
        */
        int leftmostButton = -1;
        for (int i = 0; i < buttonStatus[effectiveMonomeRow]->size(); ++i)
        {
            if (buttonStatus[effectiveMonomeRow]->getUnchecked(i))
            {
                leftmostButton = i;
                break;
            }
        }

        const int numChunks = *static_cast<const int*>
            (sampleStripArray[effectiveMonomeRow]->getSampleStripParam(SampleStrip::pNumChunks));

        if (monomeCol < numChunks)
        {

            // Check if there is a button being held to the right of the 
            // current button and if so, stop that strip.
            if (monomeCol < leftmostButton && state)
            {
                sampleStripArray[effectiveMonomeRow]->stopSamplePlaying();

                /* JUST A BIT OF FUN!
                bool isReversed = *static_cast<const bool*>
                    (sampleStripArray[effectiveMonomeRow]->getSampleStripParam(SampleStrip::pIsReversed));
                isReversed = !isReversed;
                sampleStripArray[effectiveMonomeRow]->setSampleStripParam(SampleStrip::pIsReversed, &isReversed);
                */
                DBG("strip " << effectiveMonomeRow << " stopped by combo.");
            }
            // Check if there is a button being held to the left of the 
            // current button and if so, loop the strip between those points.
            else if ((monomeCol > leftmostButton) && (leftmostButton >= 0) && state)
            {
                // TODO: this!
                DBG(leftmostButton << " left most btn");

                setSampleStripParameter(SampleStrip::pStartChunk, &leftmostButton, effectiveMonomeRow);
                setSampleStripParameter(SampleStrip::pEndChunk, &monomeCol, effectiveMonomeRow);

                // The +1 here is because midi channels start at 1 not 0!
                //if (state) monomeState.noteOn(effectiveMonomeRow + 1, monomeCol, 1.0f);
                //else monomeState.noteOff(effectiveMonomeRow + 1, monomeCol);

            }
            // else just play normally
            else
            {

                // The +1 here is because midi channels start at 1 not 0!
                if (state)
                {
                    int loopStart = 0;
                    int loopEnd = numChunks;    // select all chunks

                    setSampleStripParameter(SampleStrip::pStartChunk, &loopStart, effectiveMonomeRow);
                    setSampleStripParameter(SampleStrip::pEndChunk, &loopEnd, effectiveMonomeRow);
                    monomeState.noteOn(effectiveMonomeRow + 1, monomeCol, 1.0f);
                }
                else monomeState.noteOff(effectiveMonomeRow + 1, monomeCol);

            }

            // update the button tracking
            buttonStatus[effectiveMonomeRow]->set(monomeCol, state);
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

AudioSample * mlrVSTAudioProcessor::getAudioSample(const int &samplePoolIndex)
{
    if (samplePoolIndex >= 0 && samplePoolIndex < samplePool.size())
        return samplePool[samplePoolIndex];
    else return 0;
}
SampleStrip* mlrVSTAudioProcessor::getSampleStrip(const int &index) 
{
    jassert( index < sampleStripArray.size() );
    SampleStrip *tempStrip = sampleStripArray[index];
    return tempStrip;
}
int mlrVSTAudioProcessor::getNumSampleStrips()
{
    return sampleStripArray.size();
}

void mlrVSTAudioProcessor::switchChannels(const int &newChan, const int &stripID)
{
    // Let the strip now about the new channel
    sampleStripArray[stripID]->setSampleStripParam(SampleStrip::pCurrentChannel, &newChan);
}


/////////////////////
// Preset Handling //
/////////////////////

void mlrVSTAudioProcessor::savePreset(const String &presetName)
{
    // Create an outer XML element..
    XmlElement newPreset("PRESET");

    newPreset.setAttribute("name", presetName);

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
        if (preset->getTagName() == "PRESET_NONE")
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

/////////////////////
// Global Settings //
/////////////////////
void mlrVSTAudioProcessor::updateGlobalSetting(const int &settingID, const void *newValue)
{
    switch (settingID)
    {
    case sUseExternalTempo :
        useExternalTempo = *static_cast<const bool*>(newValue); break;
    case sNumChannels :
        numChannels = *static_cast<const int*>(newValue); break;
    case sCurrentBPM :
        {
            currentBPM = *static_cast<const double*>(newValue);
            for (int s = 0; s < sampleStripArray.size(); ++s)
                calcPlaySpeedForNewBPM(s);
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
    case sCurrentBPM : return &currentBPM;
    default : jassertfalse;
    }
}

String mlrVSTAudioProcessor::getGlobalSettingName(const int &settingID) const
{
    switch (settingID)
    {
    case sUseExternalTempo : return "use_external_tempo";
    case sNumChannels : return "num_channels";
    case sCurrentBPM : return "current_bpm";
    default : jassertfalse; return "name_not_found";
    }
}


////////////////////////////
// RECORDING / RESAMPLING //
////////////////////////////

void mlrVSTAudioProcessor::startResampling(const int &preCount, const int &numBars)
{
    resampleLength = (int) (getSampleRate() * (60.0 * numBars / currentBPM));
    resampleBuffer.setSize(2, resampleLength, false, true);
    resampleBuffer.clear();
    resamplePosition = 0;
    isResampling = true;

    DBG("resampling started");
}

void mlrVSTAudioProcessor::startRecording(const int &preCount, const int &numBars)
{
    recordLength = (int) (getSampleRate() * (60.0 * numBars / currentBPM));
    recordBuffer.setSize(2, recordLength, false, true);
    recordBuffer.clear();
    recordPosition = 0;
    isRecording = true;

    DBG("recording started");
}