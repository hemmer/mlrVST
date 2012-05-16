/*
  ==============================================================================

    timedButton.h
    Created: 5 Mar 2012 8:25:31pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __TIMEDBUTTON_H_5AC4D4C1__
#define __TIMEDBUTTON_H_5AC4D4C1__

#include "../JuceLibraryCode/JuceHeader.h"

class TimedButton  : public Button
{
public:
    TimedButton (const String& buttonName,
                 const Colour& prim,
                 const Colour& sec);

    /** Destructor. */
    ~TimedButton() { };

    void setPercentDone(float newTopValue, float newBottomValue)
    {
        if (newTopValue <= 1.0 && newTopValue >= 0.0 && newTopValue != topPercentDone)
        {
            repaint();
            topPercentDone = newTopValue;
        }

        if (newBottomValue <= 1.0 && newBottomValue >= 0.0 && newBottomValue != bottomPercentDone)
        {
            repaint();
            bottomPercentDone = newBottomValue;
        }
    }

    void setFont(Font &f, float &size)
    {
        btnFont = f;
        fontSize = size;
    }

protected:
    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown);

private:

    // Style /////////////////////////////
    Colour primaryColour, secondaryColour;
    Font btnFont;
    float fontSize;

    // Playback params ///////////////////////////////
    float topPercentDone, bottomPercentDone;

    // Check for leaks!
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimedButton);
};


#endif  // __TIMEDBUTTON_H_5AC4D4C1__
