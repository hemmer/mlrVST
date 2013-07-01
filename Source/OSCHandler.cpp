/*
  ==============================================================================

    OSCHandler.cpp
    Created: 13 Sep 2011 7:03:15pm
    Author:  Hemmer

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "OSCHandler.h"

// Constructor
OSCHandler::OSCHandler(const String &prefix, mlrVSTAudioProcessor * const owner) :
    Thread("OscListener Thread"),
    // incoming ///////////////////////////////////////
    parent(owner), incomingPort(8000),
    s(IpEndpointName("localhost", incomingPort), this),
    // incoming ///////////////////////////////////////
    buffer(), p(buffer, 1536),
    transmitSocket(IpEndpointName("localhost", 8080)),
    // strings ////////////////////////////
    OSCPrefix(prefix), ledStr(OSCPrefix + "led"), ledRowStr(OSCPrefix + "led_row"),
    ledClearStr(OSCPrefix + "clear"), buttonPressMask(OSCPrefix + "press")
{
    // setup the mask
    setPrefix(prefix);
}

void OSCHandler::buttonPressCallback(const int &monomeCol, const int &monomeRow, const bool &state)
{
    if (state) { DBG("button down " << monomeRow << " " << monomeCol); }
    else       { DBG("button up " << monomeRow << " " << monomeCol); }

    parent->processOSCKeyPress(monomeCol, monomeRow, state);
}

void OSCHandler::setLED(const int &row, const int &col, const int &val)
{
    p.Clear();
    const char * msg = ledStr.getCharPointer();
    p << osc::BeginMessage(msg) << row << col << val << osc::EndMessage;
    transmitSocket.Send(p.Data(), p.Size());
}

void OSCHandler::setRow(const int &row, const int &val)
{
    p.Clear();
    const char * msg = ledRowStr.getCharPointer();
    p << osc::BeginMessage(msg) << row << 0 << val << osc::EndMessage;
    transmitSocket.Send(p.Data(), p.Size());
}

void OSCHandler::clearGrid()
{
    p.Clear();
    const char * msg = ledClearStr.getCharPointer();
    p << osc::BeginMessage(msg) << osc::EndMessage;
    transmitSocket.Send(p.Data(), p.Size());
}

void OSCHandler::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& /*remoteEndpoint*/)
{
    const String stripWildcard = OSCPrefix + "strip/*";

    try
    {

        String msgPattern = m.AddressPattern();
        DBG("new message: " << msgPattern << " with wildcard " << stripWildcard);


        if (msgPattern.equalsIgnoreCase(OSCPrefix + "press"))
        {
            // we need three arguments for button presses
            const int numArgs = m.ArgumentCount();
            if (numArgs != 3) throw osc::Exception();

            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();

            // unpack the monome button, row and state (button up or down)
            osc::int32 row, col, state;
            args >> row >> col >> state >> osc::EndMessage;
            buttonPressCallback(row, col, state == 1);
        }
        else if (msgPattern.matchesWildcard(stripWildcard, false))
        {
            // strip off the /mlrvst/strip/ part of the message
            msgPattern = msgPattern.substring(stripWildcard.length() - 1);

            // and extract the SampleStrip rowID from the message
            const String rowIDStr = msgPattern.upToFirstOccurrenceOf("/", false, false);

            const int stripID = rowIDStr.getIntValue();

            handleStripMessage(stripID, m);
        }
    }
    catch (osc::Exception& e)
    {
        DBG("error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n");
    }
}

void OSCHandler::handleStripMessage(const int &stripID, const osc::ReceivedMessage& m)
{
    const String msgPattern = m.AddressPattern();
    const String OSCcommand = msgPattern.fromFirstOccurrenceOf("/" + String(stripID) + "/", false, false);

    DBG("strip " << stripID << " sends command: " << OSCcommand);

    if (OSCcommand == "vol")
    {
        try
        {
            const float newVol = getFloatOSCArg(m);
            parent->setSampleStripParameter(SampleStrip::pStripVolume, &newVol, stripID);
        }
        catch (osc::Exception &) { DBG("Couldn't process volume message"); }
    }


    else if (OSCcommand == "speed")
    {
        try
        {
            const float newSpeed = getFloatOSCArg(m);
            parent->setSampleStripParameter(SampleStrip::pPlaySpeed, &newSpeed, stripID);
        }
        catch (osc::Exception &) { DBG("Couldn't process speed message"); }
    }

    else if (OSCcommand == "chan")
    {
        try
        {
            const int newChannel = getIntOSCArg(m);
            parent->setSampleStripParameter(SampleStrip::pCurrentChannel, &newChannel, stripID);
        }
        catch (osc::Exception &) { DBG("Couldn't process channel message"); }

    }
}


float OSCHandler::getFloatOSCArg(const osc::ReceivedMessage& m)
{
    try
    {
        // we assume only one argument
        const int numArgs = m.ArgumentCount();
        if (numArgs != 1) throw osc::MissingArgumentException();

        osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();

        const float floatArg = arg->AsFloat();

        return floatArg;

    }
    catch (osc::WrongArgumentTypeException& )
    {
        // if the argument is an int pretending to be a float, then we
        // need to consider that too so we cast to float
        try
        {
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
            const float floatArg = (float) arg->AsInt32();

            return floatArg;
        }
        catch (osc::Exception &e)
        {
            throw e;
        }
    }
    catch (osc::Exception &e)
    {
        // pass exception on
        throw e;
    }

}

int OSCHandler::getIntOSCArg(const osc::ReceivedMessage& m)
{
    try
    {
        // we assume only one argument
        const int numArgs = m.ArgumentCount();
        if (numArgs != 1) throw osc::MissingArgumentException();

        osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();

        const int intArg = arg->AsInt32();

        return intArg;

    }
    catch (osc::Exception &e)
    {
        // pass exception on
        throw e;
    }
}