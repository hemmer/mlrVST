/*
  ==============================================================================

    PresetWindow.h
    Created: 25 Sep 2011 12:16:05pm
    Author:  Hemmr

  ==============================================================================
*/

#ifndef __PRESETWINDOW_H_F0BDFA0B__
#define __PRESETWINDOW_H_F0BDFA0B__

#include "../JuceLibraryCode/JuceHeader.h"
#include "PresetListTable.h"
#include "SetlistTable.h"

/* Forward declaration to set up pointer arrangement
   to allow sample strips to access the UI */
class mlrVSTAudioProcessor;

class PresetPanel : public Component,
                    public ButtonListener, 
                    public DragAndDropContainer
{

public:

    PresetPanel(const Rectangle<int> &bounds,
                mlrVSTAudioProcessor * const owner);

    void buttonClicked(Button *btn);
    void paint(Graphics& g);
    void visibilityChanged();

    void refreshPresetLists()
    {
        presetListTbl.loadData();
        setlistTbl.loadData();
    }

private:

    // communication //////////////////////
    mlrVSTAudioProcessor * const processor;

    // gui setup //////////////////////////
    Label panelLabel, instructionLabel;
    const float fontSize;
    const Rectangle<int> &panelBounds;
    TextButton loadSetlistBtn, saveSetlistBtn;

    PresetListTable presetListTbl;      // tables for displaying presets
    SetlistTable setlistTbl;            // and the setlist

 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel);
};






#endif  // __PRESETWINDOW_H_F0BDFA0B__
