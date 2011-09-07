/*
  ==============================================================================

    ChannelStrip
    Created: 7 Sep 2011 5:09:34pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __CHANNELSTRIP_85BD3C77__
#define __CHANNELSTRIP_85BD3C77__


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

    //ChannelStrip & operator=(const ChannelStrip &other){

    //}

    Colour getColour() const { return channelColour; }
    int getChannelNum() const { return channelNumber; }

private:
    Colour channelColour;
    int channelNumber;
};


#endif  // __CHANNELSTRIP_85BD3C77__
