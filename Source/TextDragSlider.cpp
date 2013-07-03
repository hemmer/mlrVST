/*
  ==============================================================================

    TextDragSlider.cpp
    Created: 3 Jul 2013 8:43:00pm
    Author:  hemmer

  ==============================================================================
*/

#include "TextDragSlider.h"

TextDragSlider::TextDragSlider(SliderDataType t) :
    sliderType(t),
    // UI //////////////////////////////////////////
    defaultFont("ProggyCleanTT", 18.f, Font::plain),
    backgroundColour(Colours::black),

    // value related parameters //////////////
    maxVar(1), minVar(0), valueAtDragStart(0),
    sliderValue(0), defaultValue(1)
{

}

TextDragSlider::~TextDragSlider()
{

}

void TextDragSlider::mouseDoubleClick(const MouseEvent & /*event*/)
{
    setValue(defaultValue);

    sendChangeMessage();
    repaint();
}

void TextDragSlider::mouseDown (const MouseEvent & /*event*/)
{
    valueAtDragStart = sliderValue;
}

void TextDragSlider::mouseDrag (const MouseEvent &event)
{
    // if ctrl is held, use finer increments
    const bool isCtrlHeld = (event.mods == (ModifierKeys::ctrlModifier + ModifierKeys::leftButtonModifier));

    double scale = 0.0;

    if (sliderValue.isInt())
    {
        scale = (isCtrlHeld) ? 0.5 / ((int) maxVar - (int) minVar) : 2.0 / ((int) maxVar - (int) minVar);
        var newSliderValue = (int) valueAtDragStart - (int) (event.y * scale);

        // make sure we are within limits
        if ( (int) newSliderValue > (int) maxVar)
            newSliderValue = maxVar;
        else if ( (int) newSliderValue < (int) minVar)
            newSliderValue = minVar;

        // update listeners etc
        setValue(newSliderValue);
    }
    else if (sliderValue.isDouble())
    {
        scale = (isCtrlHeld) ? 0.125 / ((double) maxVar - (double) minVar) : 0.5 / ((double) maxVar - (double) minVar);
        var newSliderValue = (double) valueAtDragStart - (double) event.y * scale;

        // make sure we are within limits
        if ( (double) newSliderValue > (double) maxVar)
            newSliderValue = maxVar;
        else if ( (double) newSliderValue < (double) minVar)
            newSliderValue = minVar;

        // update listeners etc
        setValue(newSliderValue);
    }




}

void TextDragSlider::paint(Graphics &g)
{
    const int w = getWidth();
    const int h = getHeight();

    g.fillAll(findColour(Slider::thumbColourId));

    g.setColour(findColour(Slider::thumbColourId).brighter(0.6f));
    g.drawRect(0, 0, w, h, 1);

    g.setColour(Colours::white);
    g.setFont(defaultFont);

    String sliderStr;

    if (sliderType == SliderTypeFloat)
    {
        const double sliderValueFloat = (double) sliderValue;
        sliderStr = String::formatted("%5.3f", sliderValueFloat);
    }
    else if (sliderType == SliderTypeInt)
    {
        const int sliderValueInt = (int) sliderValue;
        sliderStr = String::formatted("%2d", sliderValueInt);
    }
    else
        jassertfalse;

    g.drawFittedText(sliderStr, 5, 0, w, h, Justification::centredLeft, 2, 1.0f);
}