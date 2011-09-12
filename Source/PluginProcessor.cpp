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

#include "osc/OscOutboundPacketStream.h"
#include "ip/IpEndpointName.h"

//==============================================================================
mlrVSTAudioProcessor::mlrVSTAudioProcessor() :
delayBuffer (2, 12000),
    channelProcessorArray(),    // array of processor objects
    numChannels(4),
    channelGains(),
    oscMsgListener()
{

    DBG("starting OSC thread");
    oscMsgListener.startThread();

    // TEST: oscpack sending data
    DBG("starting OSC tests");
    char buffer[1536];
    osc::OutboundPacketStream p( buffer, 1536 );
    UdpTransmitSocket socket( IpEndpointName("localhost", 8080) );
    p.Clear();
    p << osc::BeginMessage( "/mlrvst/led" ) << 1 << 1 << 0 << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );
    DBG("finished OSC tests");
    // END TEST




    // Set up some default values..
    masterGain = 1.0f;
    delay = 0.5f;
    // add our channel processors
    buildChannelProcessorArray(numChannels);


    lastPosInfo.resetToDefault();
    delayPosition = 0;
}

void mlrVSTAudioProcessor::buildChannelProcessorArray(const int &newNumChannels)
{
    // update the number of channels
    numChannels = newNumChannels;

    // make sure we're not using the channelProcessorArray
    // while (re)building it
    suspendProcessing(true);

    // reset the gains
    channelGains.clear();
    for(int c = 0; c < maxChannels; c++) channelGains.add(0.8f);

    channelProcessorArray.clear();
    // add the list of channels 
    for(int c = 0; c < numChannels; c++)
    {
        channelProcessorArray.add(new ChannelProcessor(c, Colour((float) (0.1f * c), 0.5f, 0.5f, 1.0f)));
    }

    // resume processing
    suspendProcessing(false);
}

mlrVSTAudioProcessor::~mlrVSTAudioProcessor()
{

}

//==============================================================================
int mlrVSTAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float mlrVSTAudioProcessor::getParameter (int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
    case masterGainParam:       return masterGain;
    case delayParam:            return delay;
    case channel0GainParam:     return channelGains[0];
    case channel1GainParam:     return channelGains[1];
    case channel2GainParam:     return channelGains[2];
    case channel3GainParam:     return channelGains[3];
    case channel4GainParam:     return channelGains[4];
    case channel5GainParam:     return channelGains[5];
    case channel6GainParam:     return channelGains[6];
    case channel7GainParam:     return channelGains[7];
    default:            return 0.0f;
    }
}

void mlrVSTAudioProcessor::setParameter (int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
    case masterGainParam:       masterGain = newValue;  break;
    case delayParam:            delay = newValue;  break;
        // TODO: there might be a neater way to do this!
    case channel0GainParam:
        channelGains.set(0, newValue);    break;
    case channel1GainParam:
        channelGains.set(1, newValue);    break;
    case channel2GainParam:     
        channelGains.set(2, newValue);    break;
    case channel3GainParam:     
        channelGains.set(3, newValue);    break;
    case channel4GainParam:     
        channelGains.set(4, newValue);    break;
    case channel5GainParam:
        channelGains.set(5, newValue);    break;
    case channel6GainParam:     
        channelGains.set(6, newValue);    break;
    case channel7GainParam:     
        channelGains.set(7, newValue);    break;
    default:                    break;
    }
}

const String mlrVSTAudioProcessor::getParameterName (int index)
{
    switch (index)
    {
    case masterGainParam:       return "master gain";
    case delayParam:            return "delay";
    case channel0GainParam:     return "channel 0 gain";
    case channel1GainParam:     return "channel 1 gain";
    case channel2GainParam:     return "channel 2 gain";
    case channel3GainParam:     return "channel 3 gain";
    case channel4GainParam:     return "channel 4 gain";
    case channel5GainParam:     return "channel 5 gain";
    case channel6GainParam:     return "channel 6 gain";
    case channel7GainParam:     return "channel 7 gain";
    default:            break;
    }

    return String::empty;
}

const String mlrVSTAudioProcessor::getParameterText (int index)
{
    return String (getParameter (index), 2);
}

//==============================================================================
void mlrVSTAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // TODO: does ChannelProcessor need this?
    //synth.setCurrentPlaybackSampleRate (sampleRate);
    keyboardState.reset();
    delayBuffer.clear();
}

void mlrVSTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    keyboardState.reset();
}

void mlrVSTAudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers, etc, as it
    // means there's been a break in the audio's continuity.
    delayBuffer.clear();
}

void mlrVSTAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();
    int channel, dp = 0;

    /*  Eventually should have something like

    MonomeBuffer &monomeMessages - format (monomeRow, monomeCol, status)

    maybe strip channel 0 messages (for control options)

    */

    // for each channel, add its contributions
    // Remember to set the correct sample
    for(int c = 0; c < channelProcessorArray.size(); c++)
    {
        channelProcessorArray[c]->renderNextBlock(buffer, midiMessages, 0, numSamples);
    }

    // Apply our delay effect to the new output..
    for (channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getSampleData (channel);
        float* delayData = delayBuffer.getSampleData (jmin (channel, delayBuffer.getNumChannels() - 1));
        dp = delayPosition;

        for (int i = 0; i < numSamples; ++i)
        {
            const float in = channelData[i];
            channelData[i] += delayData[dp];
            delayData[dp] = (delayData[dp] + in) * delay;
            if (++dp > delayBuffer.getNumSamples())
                dp = 0;
        }
    }

    delayPosition = dp;

    // Go through the outgoing data, and apply our master gain to it...
    for (channel = 0; channel < getNumInputChannels(); ++channel)
        buffer.applyGain(channel, 0, buffer.getNumSamples(), masterGain);


    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());



    // ask the host for the current time so we can display it...
    AudioPlayHead::CurrentPositionInfo newTime;

    if (getPlayHead() != 0 && getPlayHead()->getCurrentPosition (newTime))
    {
        // Successfully got the current time from the host..
        lastPosInfo = newTime;
    }
    else
    {
        // If the host fails to fill-in the current time, we'll just clear it to a default..
        lastPosInfo.resetToDefault();
    }
}

//==============================================================================
AudioProcessorEditor* mlrVSTAudioProcessor::createEditor()
{
    return new mlrVSTAudioProcessorEditor (this);
}

//==============================================================================
void mlrVSTAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // Here's an example of how you can use XML to make it easy and more robust:

    // Create an outer XML element..
    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it..
    xml.setAttribute ("master gain", masterGain);
    xml.setAttribute ("delay", delay);

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void mlrVSTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != 0)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters...
            masterGain  = (float) xmlState->getDoubleAttribute ("master gain", masterGain);
            delay = (float) xmlState->getDoubleAttribute ("delay", delay);
        }
    }
}

const String mlrVSTAudioProcessor::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String mlrVSTAudioProcessor::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool mlrVSTAudioProcessor::isInputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool mlrVSTAudioProcessor::isOutputChannelStereoPair (int /*index*/) const
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
