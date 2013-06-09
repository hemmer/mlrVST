/*
    ==============================================================================

    PresetListTable.h
    Created: 28 Aug 2012 9:03:08pm
    Author:  Hemmer

    ==============================================================================
*/

#ifndef __PRESETLISTTABLE_H_6F0B8E4A__
#define __PRESETLISTTABLE_H_6F0B8E4A__

#include "../JuceLibraryCode/JuceHeader.h"

class mlrVSTAudioProcessor;


class PresetListTable : public Component,
                        public TableListBoxModel
{
public:
    
    PresetListTable(mlrVSTAudioProcessor * const owner);

    ~PresetListTable(){}
        
    // overloaded from TableListBoxModel
    int getNumRows() { return numRows; }

    void paint(Graphics & g)
    {
        g.fillAll(Colours::white);
    }

    // This is overloaded from TableListBoxModel, and should fill
    // in the background of the whole row
    void paintRowBackground (Graphics& g, int /*rowNumber*/, int /*width*/, int /*height*/, bool rowIsSelected)
    {
        if (rowIsSelected) g.fillAll (Colours::black.withAlpha(0.3f));
    }

    // This is overloaded from TableListBoxModel, and must paint any cells
    // that aren't using custom components.
    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/)
    {
        g.setColour (Colours::black);
        g.setFont (defaultFont);

        const XmlElement* rowElement = presetData->getChildElement (rowNumber);

        if (rowElement != 0)
        {
            if (columnId == 3)
            {
                const String text("REMOVE");
                g.setColour (Colours::lightgrey);
                g.fillRoundedRectangle(2.0f, 2.0f, width - 4.0f, height-4.0f, 2.0f);

                g.setColour (Colours::black);
                g.drawText (text, 2, 0, width - 4, height, Justification::horizontallyCentred, true);
            }
            if (columnId == 4)
            {
                const String text("RENAME");
                g.setColour (Colours::lightgrey);
                g.fillRoundedRectangle(2.0f, 2.0f, width - 4.0f, height-4.0f, 2.0f);

                g.setColour (Colours::black);
                g.drawText (text, 2, 0, width - 4, height, Justification::horizontallyCentred, true);
            }
            else
            {
                const String text(rowElement->getStringAttribute (getAttributeNameForColumnId (columnId)));
                g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
            }
        }

        g.setColour (Colours::black.withAlpha (0.3f));
        g.fillRect (width - 1, 0, 1, height);
    }

    // when we drag a group of rows, this records their IDs for adding to
    // the setlist
    var getDragSourceDescription (const SparseSet<int>& selectedRows)
    {
        const int numSelectedRows = selectedRows.size();

        Array<var> rowList;
        for (int i = 0; i < numSelectedRows; ++i) rowList.add(selectedRows[i]);

        return rowList;
    }


    void resized()
    {
        // position our table with a gap around its edge
        table.setBoundsInset (BorderSize<int> (5));
    }

    // click to delete preset etc
    void cellClicked (int rowNumber, int columnId, const MouseEvent &e);
    // override to select preset by double clicking
    void cellDoubleClicked (int rowNumber, int columnId, const MouseEvent &e);

    void loadData();

private:
    // communication //////////////////////
    mlrVSTAudioProcessor * const processor;

    // gui / components /////////////////////////////////
    TableListBox table;			// the table component itself
    const Font defaultFont;     // font size for rows

    // data ////////////////////////////////////////////////////////////
    XmlElement* presetData;     // pointer to the list of unique presets
    int numRows;                // The number of rows of data we've got


    // a utility method to search our XML for the attribute that matches a column ID
    const String getAttributeNameForColumnId (const int columnId) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetListTable);
};


#endif  
