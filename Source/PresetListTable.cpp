/*
  ==============================================================================

    PresetListTable.cpp
    Created: 2 Sep 2012 3:14:24pm
    Author:  Hemmer

    A table component to list the possible presets, including some additional
    information like BMP, and buttons to delete / rename.

  ==============================================================================
*/

#include "PresetListTable.h"
#include "PluginProcessor.h"

PresetListTable::PresetListTable(mlrVSTAudioProcessor * const owner) :
    // communication /////////
    processor(owner),
    // gui / components //////
    table(), defaultFont("ProggyCleanTT", 18.f, Font::plain),
    // data //////////////////
    presetData(0), numRows(0)
{
    // load any initial data from the preset list
    loadData();

    // create our table component and add it
    addAndMakeVisible (&table);
    table.setModel(this);

    // give it a border
    table.setColour(ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness(1);

    // Add some columns to the table header
    int index = 0;
    table.getHeader().addColumn("NAME", ++index, 230, 230, 230, TableHeaderComponent::visible);
    table.getHeader().addColumn("BPM", ++index, 50, 100, 100, TableHeaderComponent::visible);
    table.getHeader().addColumn("", ++index, 50, 50, 50, TableHeaderComponent::visible);
    table.getHeader().addColumn("", ++index, 50, 50, 50, TableHeaderComponent::visible);

    // we want users to be able to add multiple presets to the setlist at once
    table.setMultipleSelectionEnabled (true);
}


void PresetListTable::loadData()
{
    // get a pointer to the preset list
    presetData = &(processor->getPresetListP());
    numRows = presetData->getNumChildElements();

    // and let the table redraw
    table.updateContent();
    repaint();
}

// a utility method to search our XML for the attribute that matches a column ID
const String PresetListTable::getAttributeNameForColumnId (const int columnId) const
{
    switch (columnId)
    {
    case 1 : return GlobalSettings::getGlobalSettingName(GlobalSettings::sPresetName);
    case 2 : return GlobalSettings::getGlobalSettingName(GlobalSettings::sCurrentBPM);
    default : return String::empty;
    }
}


void PresetListTable::cellClicked (int rowNumber, int columnId, const MouseEvent &)
{
    // column 3 removes the preset
    if (columnId == 3)
    {
        processor->removePresetListItem(rowNumber);
    }

    // column 4 renames the preset
    else if (columnId == 4)
    {

#if JUCE_MODAL_LOOPS_PERMITTED

        AlertWindow w("Rename Preset", "Please enter a new name for this preset", AlertWindow::NoIcon);

        w.setColour(AlertWindow::textColourId, Colours::black);
        w.setColour(AlertWindow::outlineColourId, Colours::black);

        // get the old name of the preset
        XmlElement * rowPreset = presetData->getChildElement(rowNumber);
        const String oldName = rowPreset->getStringAttribute(GlobalSettings::getGlobalSettingName(GlobalSettings::sPresetName));

        w.addTextEditor("text", oldName, "new preset name:");
        w.addButton("ok", 1, KeyPress (KeyPress::returnKey, 0, 0));
        w.addButton("cancel", 0, KeyPress (KeyPress::escapeKey, 0, 0));

        if (w.runModalLoop() != 0) // is they picked 'ok'
        {
            // this is the text they entered..
            String newPresetName(w.getTextEditorContents("text"));

            processor->renamePreset(newPresetName, rowNumber);
        }
#endif

    }

    // refresh the table
    loadData();
}

void PresetListTable::cellDoubleClicked (int rowNumber, int, const MouseEvent &)
{
    processor->selectPresetListItem(rowNumber);
}