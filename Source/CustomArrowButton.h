#ifndef __CUSTOMARROWBUTTON__
#define __CUSTOMARROWBUTTON__

#include "CustomArrowButton.h"
#include "../JuceLibraryCode/JuceHeader.h"

class CustomArrowButton  : public Button
{
public:

    CustomArrowButton (const String& buttonName,
                 float arrowDirection,
                 const Colour &arrowColour,
                 const Colour &backgroundColour);

    ~CustomArrowButton();

    enum ColourIds
    {
        backgroundColourId,
        arrowColourId
    };

    void setColour(const int &colourID, const Colour &newColour)
    {
        switch (colourID)
        {
        case backgroundColourId : backgroundColour = newColour; break;
        case arrowColourId : arrowColour = newColour; break;
        }
    }

    void setDirection (const float &newDirection)
    {
        float difference = newDirection - direction;
        path.applyTransform (AffineTransform::rotation (float_Pi * 2.0f * difference, 0.5f, 0.5f));

        direction = newDirection;
    }

    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown);

private:
    Colour arrowColour, backgroundColour;
    float direction;
    Path path;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomArrowButton)
};


#endif
