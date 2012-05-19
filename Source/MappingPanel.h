/*
  ==============================================================================

    MappingsPanel.h
    Created: 1 May 2012 6:47:27pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __MAPPINGPANEL_H_9FE09672__
#define __MAPPINGPANEL_H_9FE09672__

#include "../JuceLibraryCode/JuceHeader.h"
#include "mlrVSTLookAndFeel.h"

/* Forward declaration to set up pointer arrangement
   to allow settings panel access to the UI */
class mlrVSTAudioProcessorEditor;


class MappingPanel : public Component,
                     public ButtonListener,
                     public ComboBoxListener
                     //public MouseListener
{
public:
    MappingPanel(const Rectangle<int> &bounds,
                 mlrVSTAudioProcessor * const owner);

    ~MappingPanel();

    void buttonClicked(Button *btn);
    void comboBoxChanged(ComboBox *box);
    void paint(Graphics& g);
    void mouseEnter (const MouseEvent &e);
    void mouseExit (const MouseEvent &e);

private:

    // Communication ///////////////////////////////
    // Pointer to parent GUI component
    mlrVSTAudioProcessor * const processor;

    // Style / layout //////////////////////////////
    Label panelLabel;           // Main header label
    menuLookandFeel menuLF;
    const float fontSize;
    const Rectangle<int> &panelBounds;
    int xPosition, yPosition;

    // Button maps /////////////////////////////
    OwnedArray<DrawableButton> buttonMatrix;
    Path monomePath;
    OwnedArray<Label> mappingLabels;
    int numCols, numRows;

    void setupHeaderLabel(Label &lbl)
    {
        addAndMakeVisible(&lbl);
        lbl.setColour(Label::backgroundColourId, Colours::black);
        lbl.setColour(Label::textColourId, Colours::white);
        lbl.setFont(fontSize);
    }
    void setupNormalLabel(Label &lbl, float factor = 1.0f)
    {
        addAndMakeVisible(&lbl);
        lbl.setColour(Label::backgroundColourId, Colours::black);
        lbl.setColour(Label::textColourId, Colours::white);
        lbl.setFont(factor * fontSize);
    }

    // check for leaks while in DEBUG mode!
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingPanel);
};

#endif  // __MAPPINGPANEL_H_9FE09672__