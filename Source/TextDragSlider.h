/*
  ==============================================================================

    TextDragSlider.h
    Created: 3 Jul 2013 8:43:00pm
    Author:  hemmer

  ==============================================================================
*/

#ifndef __TEXTDRAGSLIDER_H_B7B5D53F__
#define __TEXTDRAGSLIDER_H_B7B5D53F__

#include "../JuceLibraryCode/JuceHeader.h"


class TextDragSlider  : public Component,
                        public ChangeBroadcaster
{
public:

    // We have slightly different behaviour depending on
    // whether we are dealing with ints or doubles
    typedef enum
    {
        SliderTypeInt,
        SliderTypeFloat
    } SliderDataType ;

    TextDragSlider(SliderDataType t);
    ~TextDragSlider();

    void mouseDoubleClick(const MouseEvent &event);
    void mouseDown (const MouseEvent &event);
    void mouseDrag (const MouseEvent &event);

    void paint(Graphics &g);

    void setValue(const var &newValue, const bool doSendChangeMessage = true)
    {
        sliderValue = newValue;

        if (doSendChangeMessage) sendChangeMessage();

        repaint();
    }

    const var getValue() { return sliderValue; }

    void setMaxMin(const var &max, const var &min)
    {
        maxVar = max;
        minVar = min;
    }

    void setDefault (const var newDefaultVar) { defaultValue = newDefaultVar; }

    void setBackgroundColour(Colour &colour) { backgroundColour = colour; }

private:

    SliderDataType sliderType;

    // UI //////////////////////////////////////////
    Font defaultFont;
    Colour backgroundColour;

    // value related parameters //////////////
    var maxVar, minVar, valueAtDragStart;
    var sliderValue, defaultValue;

    JUCE_LEAK_DETECTOR(TextDragSlider);
};


#endif  // __TEXTDRAGSLIDER_H_B7B5D53F__
