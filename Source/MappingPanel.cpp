/*
==============================================================================

MappingPanel.cpp
Created: 1 May 2012 6:47:27pm
Author:  Hemmer

==============================================================================
*/

#include "PluginEditor.h"
#include "MappingPanel.h"

MappingPanel::MappingPanel(const Rectangle<int> &bounds,
    mlrVSTAudioProcessor * const owner) :
    // Communtication ////////////////////////////////
    processor(owner),
    // Style / layout ////////////////////////////////
    menuLF(), fontSize(7.4f), panelBounds(bounds),
    xPosition(0), yPosition(0),
    // Button maps ///////////////////////////////////
    topRowButtons(), normalRowButtons(), monomePath(),
    topRowMappingLabel("top row mapping:", "top row mapping:"),
    normalRowMappingLabel("normal row mapping:", "normal row mapping:"),

    panelLabel("Mapping Setup", "Mapping Setup")
{
    const int labelWidth = 246;
    // setup for virtual monome
    const int numCols = 8;
    const float monomeWidth = (float) (panelBounds.getWidth() - labelWidth - 3*PAD_AMOUNT);
    // size of monome buttons in pixels
    const int buttonSize = (int) ( (monomeWidth - (numCols+1)*PAD_AMOUNT) / (float) numCols );
    const float monomeHeight = 2.0f * buttonSize + 3.0f*PAD_AMOUNT;
    // dimensions of rowMappingLabel
    const int labelHeight = (int) (monomeHeight / 2.0f);

    // setup the main panel label
    panelLabel.setBounds(xPosition, yPosition, panelBounds.getWidth(), 30);
    setupNormalLabel(panelLabel, 2.0);
    xPosition = PAD_AMOUNT;
    yPosition += 30 + PAD_AMOUNT;

    // row mapping stuff ///////////////////////////////////////////
    // these are for displaying the mapping of the button currently
    // being hovered over
    topRowMappingLabel.setBounds(xPosition, yPosition, labelWidth, labelHeight);
    setupNormalLabel(topRowMappingLabel);
    yPosition += labelHeight;
    normalRowMappingLabel.setJustificationType(Justification::centred);
    normalRowMappingLabel.setBounds(xPosition, yPosition, labelWidth, (int) (monomeHeight - labelHeight));
    setupNormalLabel(normalRowMappingLabel);
    xPosition += labelWidth + PAD_AMOUNT;
    yPosition = 40;

    // add the faux monome + buttons
    monomePath.addRoundedRectangle((float) xPosition, (float) (yPosition),
                                   monomeWidth, monomeHeight, (float) PAD_AMOUNT);
    xPosition += PAD_AMOUNT;
    yPosition += PAD_AMOUNT;
    Path buttonPath;
    buttonPath.addRoundedRectangle(0.0f, 0.0f, (float) buttonSize,
                                               (float) buttonSize,
                                               (float) buttonSize / 5.0f);
    // create actual images for the buttons
    DrawablePath normal, over;
    normal.setPath(buttonPath);
    normal.setFill(Colours::white);
    over.setPath(buttonPath);
    over.setFill(Colours::orange);

    // add the rows of buttons
    for (int b = 0; b < numCols; ++b)
    {
        topRowButtons.add(new DrawableButton("top row", DrawableButton::ImageRaw));
        DrawableButton *latestBtn = topRowButtons.getLast();
        latestBtn->setImages(&normal, &over);
        latestBtn->addListener(this);
        latestBtn->setBounds(xPosition, yPosition, buttonSize, buttonSize);
        latestBtn->addMouseListener(this, false);
        latestBtn->setTriggeredOnMouseDown(true);
        addAndMakeVisible(latestBtn);

        yPosition += (int) buttonSize + PAD_AMOUNT;

        normalRowButtons.add(new DrawableButton("normal row", DrawableButton::ImageRaw));
        latestBtn = normalRowButtons.getLast();
        latestBtn->setImages(&normal, &over);
        latestBtn->addListener(this);
        latestBtn->setBounds(xPosition, yPosition, buttonSize, buttonSize);
        latestBtn->addMouseListener(this, false);
        latestBtn->setTriggeredOnMouseDown(true);
        addAndMakeVisible(latestBtn);

        xPosition += PAD_AMOUNT + buttonSize;
        yPosition -= (int) buttonSize + PAD_AMOUNT;
    }

    // update positions
    xPosition = PAD_AMOUNT;
    yPosition += buttonSize + PAD_AMOUNT;
}

void MappingPanel::buttonClicked(Button *btn)
{
    // check if any of the top buttons have been clicked
    for (int b = 0; b < topRowButtons.size(); ++b)
    {
        // if so display the menu of possible mappings
        if (btn == topRowButtons[b])
        {
            PopupMenu topRowMappingMenu;
            const int numOptions = processor->numTopRowMappings;

            // add the list of mappings (+1 is because 0 reserved for no click)
            for (int m = 0; m < numOptions; ++m)
                topRowMappingMenu.addItem(m + 1, processor->getTopRowMappingName(m));

            // show the menu and store choice
            int mappingChoice = topRowMappingMenu.showMenu(PopupMenu::Options().withTargetComponent(btn));

            if (mappingChoice > 0 && mappingChoice <= numOptions)
            {
                mappingChoice--;
                processor->setTopRowMapping(b, mappingChoice);
                const String newMappingName = processor->getTopRowMappingName(mappingChoice);

                topRowButtons[b]->setButtonText(newMappingName);
                topRowButtons[b]->setTooltip(newMappingName);
                topRowMappingLabel.setText("top row mapping: " + newMappingName, false);
            }
            return;
        }
    }

    // check if any of the normal row buttons have been clicked
    for (int b = 0; b < normalRowButtons.size(); ++b)
    {
        // if so display the menu of possible mappings
        if (btn == normalRowButtons[b])
        {
            PopupMenu normalRowMappingMenu;
            const int numOptions = processor->numNormalRowMappings;

            // add the list of mappings (+1 is because 0 reserved for no click)
            for (int m = 0; m < numOptions; ++m)
                normalRowMappingMenu.addItem(m + 1, processor->getNormalRowMappingName(m));

            // show the menu and store choice
            int mappingChoice = normalRowMappingMenu.showMenu(PopupMenu::Options().withTargetComponent(btn));

            if (mappingChoice > 0 && mappingChoice <= numOptions)
            {
                mappingChoice--;
                processor->setNormalRowMapping(b, mappingChoice);
                const String newMappingName = processor->getNormalRowMappingName(mappingChoice);

                normalRowButtons[b]->setButtonText(newMappingName);
                normalRowButtons[b]->setTooltip(newMappingName);
                normalRowMappingLabel.setText("normal row mapping: " + newMappingName, false);
            }
            return;
        }
    }
}

void MappingPanel::comboBoxChanged(ComboBox * /*box*/)
{

}

void MappingPanel::paint(Graphics &g)
{
    g.fillAll(Colours::grey.withAlpha(0.9f));

    g.setColour(Colours::black.withAlpha(0.5f));
    g.fillPath(monomePath);
}

void MappingPanel::mouseEnter (const MouseEvent &e)
{
    // cast the component that the mouse entered to
    // a DrawableButton to check if it was one of the
    // mapping buttons
    DrawableButton *currentBtn = static_cast< DrawableButton * >(e.eventComponent);

    // check if the mouse has wandered onto any of
    // the top row buttons
    for (int b = 0; b < topRowButtons.size(); ++b)
    {
        // if so display the mapping for that button
        if (currentBtn == topRowButtons[b])
        {
            const int currentMapping = processor->getTopRowMapping(b);
            String mappingName = processor->getTopRowMappingName(currentMapping);
            topRowMappingLabel.setText("top row mapping: " + mappingName, false);
            return;
        }
    }

    // otherwise check if the mouse has wandered onto any of
    // the normal row buttons
    for (int b = 0; b < normalRowButtons.size(); ++b)
    {
        // if so display the mapping for that button
        if (currentBtn == normalRowButtons[b])
        {
            const int currentMapping = processor->getNormalRowMapping(b);
            String mappingName = processor->getNormalRowMappingName(currentMapping);
            normalRowMappingLabel.setText("normal row mapping: " + mappingName, false);
            return;
        }
    }
}

void MappingPanel::mouseExit (const MouseEvent &e)
{
    // cast the component that the mouse entered to
    // a DrawableButton to check if it was one of the
    // mapping buttons
    DrawableButton *currentBtn = static_cast< DrawableButton * >(e.eventComponent);

    // check if the mouse has wandered onto any of
    // the top row buttons
    for (int b = 0; b < topRowButtons.size(); ++b)
    {
        // if so display the mapping for that button
        if (currentBtn == topRowButtons[b])
        {
            topRowMappingLabel.setText("top row mapping:", false);
            return;
        }
    }

    // otherwise check if the mouse has wandered onto any of
    // the normal row buttons
    for (int b = 0; b < normalRowButtons.size(); ++b)
    {
        // if so display the mapping for that button
        if (currentBtn == normalRowButtons[b])
        {
            normalRowMappingLabel.setText("normal row mapping:", false);
            return;
        }
    }
}