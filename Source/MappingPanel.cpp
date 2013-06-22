/*
==============================================================================

MappingPanel.cpp
Created: 1 May 2012 6:47:27pm
Author:  Hemmer

==============================================================================
*/

#include "mlrVSTGUI.h"
#include "MappingPanel.h"

MappingPanel::MappingPanel(const Rectangle<int> &bounds,
    mlrVSTAudioProcessor * const owner) :
    // Communtication ////////////////////////////////
    processor(owner),
    // Style / layout ////////////////////////////////
    panelLabel("Mapping Setup", "mapping setup"),
    menuLF(), defaultFont("ProggyCleanTT", 18.f, Font::plain), panelBounds(bounds),
    xPosition(0), yPosition(0),
    // Button maps ///////////////////////////////////
    buttonMatrix(), monomePath(),
    mappingLabels(),
    numCols(8), numRows(3)     // TODO: this should be obtained from PluginProcessor

{
    const int labelWidth = 246;
    // setup for virtual monome
    const float monomeWidth = (float) (panelBounds.getWidth() - labelWidth - 3*PAD_AMOUNT);
    // size of monome buttons in pixels
    const int buttonSize = (int) ( (monomeWidth - (numCols+1)*PAD_AMOUNT) / (float) numCols );
    const float monomeHeight = (float) (numRows * buttonSize + (numRows+1)*PAD_AMOUNT);
    // dimensions of rowMappingLabel
    const int labelHeight = (int) (monomeHeight / numRows);

    // setup the main panel label
    panelLabel.setBounds(xPosition, yPosition, panelBounds.getWidth(), 36);
    setupNormalLabel(panelLabel, 2.0f);
    xPosition = PAD_AMOUNT;
    yPosition += 30 + PAD_AMOUNT;


    // row mapping stuff /////////////////////////
    // these are for displaying the mapping of the
    // button currently being hovered over
    for (int r = 0; r < numRows; ++r)
    {
        mappingLabels.add(new Label());
        Label *latestLbl = mappingLabels.getLast();
        latestLbl->setBounds(xPosition, yPosition, labelWidth, labelHeight);
        latestLbl->setText(((r==0) ? "Top row: " : "Modifier " + String(r) + ": "), NotificationType::dontSendNotification);
        setupNormalLabel(*latestLbl);

        yPosition += labelHeight;
    }
    xPosition = 2 * PAD_AMOUNT + labelWidth;
    yPosition -= numRows * labelHeight;


    ////////////////////////////////
    // add the faux monome + buttons
    monomePath.addRoundedRectangle((float) xPosition, (float) (yPosition),
                                   monomeWidth, monomeHeight, (float) PAD_AMOUNT);
    xPosition += PAD_AMOUNT;
    yPosition += PAD_AMOUNT;

    Path buttonPath;
    buttonPath.addRoundedRectangle(0.0f, 0.0f, (float) buttonSize,
                                   (float) buttonSize, (float) buttonSize / 5.0f);
    // create actual images for the buttons
    DrawablePath normal, over;
    normal.setPath(buttonPath);
    normal.setFill(Colours::white);
    over.setPath(buttonPath);
    over.setFill(Colours::orange);


    // and add the rows of buttons to the GUI
    for (int r = 0; r < numRows; ++r)
    {
        for (int b = 0; b < numCols; ++b)
        {
            buttonMatrix.add(new DrawableButton("button", DrawableButton::ImageRaw));
            DrawableButton *latestBtn = buttonMatrix.getLast();
            latestBtn->setImages(&normal, &over);
            latestBtn->addListener(this);
            latestBtn->setBounds(xPosition, yPosition, buttonSize, buttonSize);
            latestBtn->addMouseListener(this, false);
            latestBtn->setTriggeredOnMouseDown(true);
            addAndMakeVisible(latestBtn);

            xPosition += PAD_AMOUNT + buttonSize;
        }

        // reset positions for next row
        xPosition -= (PAD_AMOUNT + buttonSize) * numCols;
        yPosition += buttonSize + PAD_AMOUNT;
    }

}

MappingPanel::~MappingPanel()
{
    buttonMatrix.clear(true);
}

void MappingPanel::buttonClicked(Button *btn)
{
    // check if any of the monome buttons have been clicked
    for (int r = 0; r < numRows; ++r)
        for (int c = 0; c < numCols; ++c)
        {
            const int index = r * numCols + c;
            // if so display the menu of possible mappings
            if (btn == buttonMatrix.getUnchecked(index))
            {
                PopupMenu mappingMenu;
                int numOptions;

                // top row mappings
                if (r == 0)
                {
                    numOptions = processor->numTopRowMappings;

                    // add the list of mappings (+1 is because 0 reserved for no click)
                    for (int m = 0; m < numOptions; ++m)
                        mappingMenu.addItem(m + 1, processor->getTopRowMappingName(m));
                }
                // normal row mappings
                else
                {
                    numOptions = processor->numNormalRowMappings;

                    // add the list of mappings (+1 is because 0 reserved for no click)
                    for (int m = 0; m < numOptions; ++m)
                        mappingMenu.addItem(m + 1, processor->getNormalRowMappingName(m));
                }

                // show the menu and store choice
                int mappingChoice = mappingMenu.showMenu(PopupMenu::Options().withTargetComponent(btn));

                if (mappingChoice > 0 && mappingChoice <= numOptions)
                {

                    mappingChoice--;
                    String newMappingName;
                    if (r == 0)
                    {
                        processor->setMonomeMapping(mlrVSTAudioProcessor::rmTopRowMapping, c, mappingChoice);
                        newMappingName = processor->getTopRowMappingName(mappingChoice);
                    }
                    else if (r == 1 || r == 2)
                    {
                        const int modifierBtnType = r - 1;
                        processor->setMonomeMapping(modifierBtnType, c, mappingChoice);
                        newMappingName = processor->getNormalRowMappingName(mappingChoice);
                    }

                    btn->setButtonText(newMappingName);
                    btn->setState(Button::buttonNormal);
                    mappingLabels[r]->setText(((r==0) ? "Top row: " : "Modifier " + String(r) + ": " + newMappingName), NotificationType::dontSendNotification);
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
    // cast the component that the mouse entered to a DrawableButton
    // to check if it was one of the mapping buttons
    DrawableButton *currentBtn = static_cast< DrawableButton * >(e.eventComponent);

    // check if the mouse has wandered onto any of fake monome buttons
    for (int r = 0; r < numRows; ++r)
        for (int c = 0; c < numCols; ++c)
        {
            const int index = r * numCols + c;
            // if so display the mapping for that button
            if (currentBtn == buttonMatrix.getUnchecked(index))
            {
                if (r == 0)
                {
                    const int currentMapping = processor->getMonomeMapping(mlrVSTAudioProcessor::rmTopRowMapping, c);
                    String mappingName = processor->getTopRowMappingName(currentMapping);
                    mappingLabels[r]->setText("Top row: " + mappingName, NotificationType::dontSendNotification);
                }
                else if (r == 1 || r == 2)
                {
                    // modifierBtn = 0 - buttonA
                    // modifierBtn = 1 - buttonB
                    const int modifierBtn = r - 1;
                    const int currentMapping = processor->getMonomeMapping(modifierBtn, c);

                    String mappingName = processor->getNormalRowMappingName(currentMapping);
                    mappingLabels[r]->setText("Modifier " + String(r) + ": " + mappingName, NotificationType::dontSendNotification);
                }
                return;
            }
        }


}

void MappingPanel::mouseExit (const MouseEvent &e)
{
    // cast the component that the mouse left to a DrawableButton
    // to check if it was one of the mapping buttons
    DrawableButton *currentBtn = static_cast< DrawableButton * >(e.eventComponent);

    // check if the mouse has wandered off any of the buttons
    for (int r = 0; r < numRows; ++r)
        for (int c = 0; c < numCols; ++c)
        {
            const int index = r * numCols + c;
            // if so hide the mappings
            if (currentBtn == buttonMatrix.getUnchecked(index))
            {
                mappingLabels[r]->setText(((r==0) ? "Top row: " : "Modifier " + String(r) + ": "), NotificationType::dontSendNotification);
                return;
            }
        }
}