/*
  ==============================================================================

    OSCHandler.cpp
    Created: 13 Sep 2011 7:03:15pm
    Author:  Hemmer

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "OSCHandler.h"

void OSCHandler::sendMessage(const int &a1, const int &a2, const int &a3)
{
    parent->processOSCMessage(a1, a2, a3);
}

