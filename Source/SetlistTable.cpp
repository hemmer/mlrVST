/*
  ==============================================================================

    SetlistTable.cpp
    Created: 2 Sep 2012 3:14:24pm
    Author:  Hemmer

  ==============================================================================
*/

#include "SetlistTable.h"
#include "PluginProcessor.h"


SetlistTable::SetlistTable(mlrVSTAudioProcessor * const owner, 
                           PresetListTable * presetListTbl_) :
    // communication ///////////////////////////////
    processor(owner), presetListTbl(presetListTbl_),
    // gui / components ////////////////////////////
    table(), font (7.4f), insertAtIndex(-1),
    // data ////////////////////////////////////////
    setlistData(0), numRows(0)
{
    // load the setlist (if it exists)
    loadData();

    // create our table component and add it
    addAndMakeVisible (&table);
    table.setModel (this);

    // give it a border
    table.setColour (ListBox::outlineColourId, Colours::grey);
    table.setOutlineThickness (1);

    // Add some columns to the table header
    int index = 0;
    table.getHeader().addColumn("#", ++index, 50, 50, 50, TableHeaderComponent::visible);
    table.getHeader().addColumn("NAME", ++index, 170, 250, 250, TableHeaderComponent::visible);
    table.getHeader().addColumn("", ++index, 50, 50, 50, TableHeaderComponent::visible);
}

void SetlistTable::loadData()
{
    // get a pointer to the setlist data
    setlistData = &(processor->getSetlistP());
    numRows = setlistData->getNumChildElements();

    table.updateContent();
    repaint();
}

// a utility method to search our XML for the attribute that matches a column ID
const String SetlistTable::getAttributeNameForColumnId (const int columnId) const
{
    if (columnId == 2) return processor->getGlobalSettingName(mlrVSTAudioProcessor::sPresetName);
    else return String::empty;
    
}

void SetlistTable::cellClicked (int rowNumber, int columnId, const MouseEvent &)
{
    // if we are removing this setlist item
    if (columnId == 3)
    {
        processor->removeSetlistItem(rowNumber);

        // refresh the table
        loadData();
    }

}

void SetlistTable::cellDoubleClicked (int rowNumber, int, const MouseEvent &)
{
    processor->selectSetlistItem(rowNumber);
}

void SetlistTable::itemDropped (const SourceDetails& dragSourceDetails)
{
    var rowList = dragSourceDetails.description;
    const int newNumRows = rowList.size();

    for (int i = 0; i < newNumRows; ++i)
    {
        const int presetID = rowList[i];

        // NOTE: the + i here preserves the order in which they were dragged
        processor->insetPresetIntoSetlist(presetID, insertAtIndex+i);
    }

    // the dragging operation has finished
    insertAtIndex = -1;
    loadData();
}