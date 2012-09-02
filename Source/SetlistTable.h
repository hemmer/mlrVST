/*
  ==============================================================================

    SetlistTable.h
    Created: 2 Sep 2012 3:14:24pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __SETLISTTABLE_H_6F0B8E4A__
#define __SETLISTTABLE_H_6F0B8E4A__


#include "../JuceLibraryCode/JuceHeader.h"
#include "PresetListTable.h"

class mlrVSTAudioProcessor;

class SetlistTable    : public Component,
                        public TableListBoxModel,
                        public DragAndDropTarget
{
public:
    
    SetlistTable(mlrVSTAudioProcessor * const owner, PresetListTable * presetListTbl_);
    ~SetlistTable() {}
        
    // overloaded from TableListBoxModel
    int getNumRows() { return numRows; }

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground (Graphics& g, int /*rowNumber*/, int /*width*/, int /*height*/, bool rowIsSelected)
    {
        if (rowIsSelected) g.fillAll (Colours::black.withAlpha(0.3f));
    }

    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    void paintCell (Graphics& g, int rowNumber, int columnId, int width, int height,
                    bool /*rowIsSelected*/)
    {
        g.setColour (Colours::black);
        g.setFont (font);

        const XmlElement* rowElement = setlistData->getChildElement (rowNumber);

        if (rowElement != 0)
        {
            // if it is the ID number
            if (columnId == 1)
            {
                const String text(rowNumber);
                g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
            }
            else if (columnId == 3)
            {
                const String text("REMOVE");
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

        // if presets are being dragged, draw a line marking where they would be placed
        if (rowNumber == insertAtIndex) g.drawLine(0.0f, 0.0f, width, 0.0f, 2.0f);
        if (rowNumber == insertAtIndex-1) g.drawLine(0.0f, height, width, height, 2.0f);
    }


    void paint(Graphics & g)
    {
        // if something is being dragged over, colour the border
        if (insertAtIndex != -1) g.fillAll(Colours::black.withAlpha(0.4f));
        else g.fillAll(Colours::white);
    }

    bool isInterestedInDragSource (const SourceDetails& dragSourceDetails)
    {
        // if the drag has come from the preset list, we accept
        // NOTE: the table itself is the parent component, we
        // only are passed the row itself, so we use getParentComponent()
        if (dragSourceDetails.sourceComponent->getParentComponent() == presetListTbl) return true;
        else return false;
    }

    // use these to track which row we should be inserting at
    void itemDragEnter (const SourceDetails& dragSourceDetails)
    {
        const int yPos = dragSourceDetails.localPosition.getY();
        insertAtIndex = table.getInsertionIndexForPosition(0, yPos);
        repaint();
    }
    void itemDragMove (const SourceDetails& dragSourceDetails)
    {
        const int yPos = dragSourceDetails.localPosition.getY();
        insertAtIndex = table.getInsertionIndexForPosition(0, yPos);
        repaint();
    }
    void itemDragExit (const SourceDetails& /*dragSourceDetails*/)
    {
        insertAtIndex = -1;
        repaint();
    }

    void itemDropped (const SourceDetails& dragSourceDetails);

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
    PresetListTable * presetListTbl;

    // gui / components /////////////
    TableListBox table;     // the table component itself
    Font font;
    // stores which row presets would be added whilst dragged over
    // NOTE: value of -1 means no presets are currently being dragged
    int insertAtIndex;      

    // data //////////////////////////////////////////
    XmlElement* setlistData;    // pointer to setlist
    int numRows;                // The number of rows of data we've got


    // (a utility method to search our XML for the attribute that matches a column ID)
    const String getAttributeNameForColumnId (const int columnId) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SetlistTable);
};



#endif  // __SETLISTTABLE_H_6F0B8E4A__
