
#include "CustomArrowButton.h"

CustomArrowButton::CustomArrowButton (const String& name,
                                float arrowDirectionInRadians,
                                const Colour &arrowCol,
                                const Colour &backgroundCol)
    : Button (name),
    arrowColour(arrowCol), backgroundColour(backgroundCol),
    direction(arrowDirectionInRadians), path()
{
    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (float_Pi * 2.0f * direction, 0.5f, 0.5f));
}

CustomArrowButton::~CustomArrowButton() {}

void CustomArrowButton::paintButton (Graphics& g, bool /*isMouseOverButton*/, bool isButtonDown)
{
    Colour colour1, colour2;

    if (isButtonDown)
    {
        colour1 = backgroundColour;
        colour2 = arrowColour;
    }
    else
    {
        colour1 = arrowColour;
        colour2 = backgroundColour;
    }

    g.fillAll(colour2);

    Path p (path);
    p.applyTransform (AffineTransform::scale(0.5f, 0.5f, 0.5f, 0.5f));

    const float offset = 1.0f;
    p.applyTransform (path.getTransformToScaleToFit (offset, offset, getWidth() - 3.0f, getHeight() - 3.0f, true));

    g.setColour (colour1);
    g.fillPath (p);
}