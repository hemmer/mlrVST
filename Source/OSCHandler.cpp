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
