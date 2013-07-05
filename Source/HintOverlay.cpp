/*
==============================================================================

    HintOverlay.cpp
    Created: June 2013
    Author:  Hemmer

    Draws a semi-transparent overlay with hints to what each mapping does.

==============================================================================
*/

#include "HintOverlay.h"
#include "PluginProcessor.h"
#include "mlrVSTGUI.h"

HintOverlay::HintOverlay(mlrVSTAudioProcessor * const owner) :
    // communication ///////////////////////
    processor(owner),
    // gui / components ////////////////////
    overlayPaintBounds(0, 0, GUI_WIDTH, 150)
{
    processor->addChangeListener(this);
}

HintOverlay::~HintOverlay()
{
    processor->removeChangeListener(this);
}

void HintOverlay::paint(Graphics &g)
{
    // see which button has been pressed
    const int modifierStatus = processor->getModifierBtnState();

    // if no button is pressed then don't display the hint
    if (modifierStatus == mlrVSTAudioProcessor::rmNoBtn) return;

    // start with the background colour
    g.setColour(Colours::black.withAlpha(0.5f));
    g.fillRect(overlayPaintBounds);

    // TODO: should be not hardcoded.
    const int numMappings = 8;


    int buttonPadding;

    if (numMappings > 8)
    {
        buttonPadding = PAD_AMOUNT/2;
        g.setFont(Font("Verdana", 12.f, Font::plain));
    }
    else
    {
        buttonPadding = PAD_AMOUNT;
        g.setFont(Font("Verdana", 16.f, Font::plain));
    }

    const int buttonSize = (overlayPaintBounds.getWidth() - (numMappings + 1) * buttonPadding) / numMappings;


    for (int i = 0; i < numMappings; ++i)
    {
        const float xPos = (float) ((buttonPadding + buttonSize)*i + buttonPadding);
        const float yPos = (overlayPaintBounds.getHeight() - buttonSize) / 2.0f;

        // highlight any mappings that are held
        if (processor->isColumnHeld(i))
            g.setColour(Colours::orange);
        else
            g.setColour(Colours::white.withAlpha(0.9f));

        g.fillRoundedRectangle(xPos, yPos, (float) buttonSize, (float) buttonSize, buttonSize/10.0f);

        if (i > 7) break;

        const int currentMapping = processor->getMonomeMapping(modifierStatus, i);
        String mappingName;

        switch (modifierStatus)
        {
        case mlrVSTAudioProcessor::rmNormalRowMappingBtnA :
        case mlrVSTAudioProcessor::rmNormalRowMappingBtnB :
            mappingName = processor->getSampleStripMappingName(currentMapping);
            break;

        case mlrVSTAudioProcessor::rmPatternBtn :
            mappingName = processor->getPatternStripMappingName(currentMapping);
            break;

        case mlrVSTAudioProcessor::rmGlobalMappingBtn :
            mappingName = processor->getGlobalMappingName(currentMapping);
            break;

        case mlrVSTAudioProcessor::rmNoBtn : break;
        default : jassertfalse;

        }

        g.setColour(Colours::black);

        //g.drawMultiLineText(mappingName, xPos+2, yPos+10, buttonSize);
        g.drawFittedText(mappingName, (int) (xPos) + 1, (int) (yPos) + 1,
            buttonSize-2, buttonSize-2, Justification::centred, 4, 1.0f);

    }
}