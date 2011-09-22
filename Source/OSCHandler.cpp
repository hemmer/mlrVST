/*
  ==============================================================================

    OSCHandler.cpp
    Created: 13 Sep 2011 7:03:15pm
    Author:  Hemmer

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "OSCHandler.h"

void OSCHandler::sendMessage(const int &monomeCol, const int &monomeRow, const int &state)
{
    parent->processOSCKeyPress(monomeCol, monomeRow, state);
}

void OSCHandler::setLED(const int &row, const int &col, const int &val)
{
    p.Clear();
    p << osc::BeginMessage("/mlrvst/led") << row << col << val << osc::EndMessage;
    transmitSocket.Send(p.Data(), p.Size());
}

void OSCHandler::clearGrid()
{
    p.Clear();
    p << osc::BeginMessage("/mlrvst/clear") << osc::EndMessage;
    transmitSocket.Send(p.Data(), p.Size());
}

void OSCHandler::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& /*remoteEndpoint*/)
{
    try
    {
        osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
        osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();

        if (strcmp(m.AddressPattern(), "/mlrvst/press") == 0)
        {
            // example #1:
            osc::int32 a1, a2, a3;
            args >> a1 >> a2 >> a3 >> osc::EndMessage;
            sendMessage(a1, a2, a3);
            DBG("/mlrvst/press " << a1 << " " << a2 << " " << a3);
        }

    }
    catch (osc::Exception& e)
    {
        DBG("error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n");
    }
}

