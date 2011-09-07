/*
  ==============================================================================

    ChannelStrip.h
    Created: 7 Sep 2011 9:35:22pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __CHANNELSTRIP_H_B6000B1__
#define __CHANNELSTRIP_H_B6000B1__


class ChannelStrip {
public:
    ChannelStrip(const Colour &col, const int &channelNum) :
        channelNumber(channelNum),
        channelColour(col)
    {
    }

    ChannelStrip() :
        channelNumber(-1),
        channelColour(Colours::white)
    {
    }

    Colour getColour() const { return channelColour; }
    int getChannelNum() const { return channelNumber; }

private:
    Colour channelColour;
    int channelNumber;
};



#endif  // __CHANNELSTRIP_H_B6000B1__
