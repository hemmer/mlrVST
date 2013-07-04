/*
  ==============================================================================

    PresetListTable.cpp
    Created: 2 Sep 2012 3:14:24pm
    Author:  Hemmer

    A table component to list the possible presets, including some additional
    information like BMP, and buttons to delete / rename.

  ==============================================================================
*/

#include "HintOverlay.h"
#include "PluginProcessor.h"
#include "mlrVSTGUI.h"

HintOverlay::HintOverlay(mlrVSTAudioProcessor * const owner) :
    // communication /////////
    processor(owner),
    // gui / components //////
	overlayPaintBounds(0, 0, GUI_WIDTH, 150)
{

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

        g.setColour(Colours::white.withAlpha(0.9f));
		g.fillRoundedRectangle(xPos, yPos, (float) buttonSize, (float) buttonSize, buttonSize/10.0f);

		if (i > 7) break;

		const int currentMapping = processor->getMonomeMapping(modifierStatus, i);
		String mappingName;

		if (modifierStatus == mlrVSTAudioProcessor::rmNormalRowMappingBtnA ||
			modifierStatus == mlrVSTAudioProcessor::rmNormalRowMappingBtnB )
		{
			mappingName = processor->getNormalRowMappingName(currentMapping);
		}
		else if (modifierStatus == mlrVSTAudioProcessor::rmPatternBtn)
		{
			mappingName = processor->getPatternRowMappingName(currentMapping);
		}


		g.setColour(Colours::black);

		//g.drawMultiLineText(mappingName, xPos+2, yPos+10, buttonSize);
		g.drawFittedText(mappingName, (int) (xPos) + 1, (int) (yPos) + 1,
			buttonSize-2, buttonSize-2, Justification::centred, 4, 1.0f);

	}
}