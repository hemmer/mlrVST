/*
  ==============================================================================

    timedButton.cpp
    Created: 5 Mar 2012 8:25:31pm
    Author:  Hemmer

    A simple button class that can display a progress bar. Useful for recording
    and resampling buttons as it lets the user know when recording is finishing.

  ==============================================================================
*/


#include "timedButton.h"
#include "mlrVSTLookAndFeel.h"


//==============================================================================
TimedButton::TimedButton (const String& name,
                          const Colour& primaryCol,
                          const Colour& secondaryCol)
: Button (name),
    // Style /////////////////////////////
    primaryColour(primaryCol), secondaryColour(secondaryCol),
    btnFont(1.0f), fontSize(1.0f),

    // Playback params ///////////////////////////////
    topPercentDone(1.0f), bottomPercentDone(1.0f)

{

}

void TimedButton::paintButton (Graphics& g, bool /*isMouseOverButton*/,
                               bool isButtonDown)
{
    g.setFont(btnFont);
    g.setFont(fontSize);

    g.setColour(isButtonDown ? primaryColour : secondaryColour);
    g.fillRect(0, 0, getWidth(), getHeight());

    g.setColour( (!isButtonDown ? primaryColour : secondaryColour).withAlpha(0.6f));
    g.fillRect(0, 0, (int) (topPercentDone * getWidth()), 5);
    g.setColour(!isButtonDown ? primaryColour : secondaryColour);
    g.drawFittedText(getButtonText(), 4, 0, getWidth(), getHeight(), Justification::centredLeft, 1);

    g.setColour(Colours::red.darker());
    g.fillRect(0, getHeight() - 5, (int) (bottomPercentDone * getWidth()), 5);
}