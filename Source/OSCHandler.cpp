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
    String msgPattern = m.AddressPattern();
    DBG("new message: " << msgPattern);

    try
    {
        osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
        osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();

        if (msgPattern == OSCPrefix + "press")
        {
            // example #1:
            osc::int32 row, col, state;
            args >> row >> col >> state >> osc::EndMessage;
            buttonPressCallback(row, col, state == 1);
            DBG(OSCPrefix << "press " << row << " " << col << " " << state);
        }
        else if (msgPattern.matchesWildcard("/mlrvst/strip/*", false))
        {
            msgPattern = msgPattern.substring(14);

            String rowIDStr = msgPattern.upToFirstOccurrenceOf("/", false, false);
            const int stripID = rowIDStr.getIntValue();

            processStripMessage(stripID, m);

            String OSCcommand = msgPattern.fromFirstOccurrenceOf(rowIDStr + "/", false, false);

            DBG("strip " << stripID << " sends command: " << OSCcommand);

            if (OSCcommand == "vol")
            {
                float newVol;
                args >> newVol >> osc::EndMessage;
                DBG(newVol);

                parent->setSampleStripParameter(SampleStrip::pStripVolume, &newVol, stripID);

            }
        }
    }
    catch (osc::Exception& e)
    {
        DBG("error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n");
    }
}

void OSCHandler::handleStripMessage(const int &stripID, osc::ReceivedMessage& m)
{

}