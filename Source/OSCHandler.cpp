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

