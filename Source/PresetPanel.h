/*
  ==============================================================================

    PresetWindow.h
    Created: 25 Sep 2011 12:16:05pm
    Author:  Hemmer

  ==============================================================================
*/

#ifndef __PRESETWINDOW_H_F0BDFA0B__
#define __PRESETWINDOW_H_F0BDFA0B__

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"


/* Forward declaration to set up pointer arrangement 
   to allow sample strips to access the UI */
class mlrVSTAudioProcessorEditor;


class PresetPanel : 
    public Component,
    public ButtonListener
{

public:

    PresetPanel(const Rectangle<int> &bounds, mlrVSTAudioProcessorEditor * const owner);

    void buttonClicked(Button *btn);
    void paint(Graphics& g);

private:

    // Pointer to parent GUI component
    mlrVSTAudioProcessorEditor * const mlrVSTEditor;

    void addRow();
    void deleteRow(const int &index);
    void arrangeButtons();

    Label panelLabel;

    OwnedArray<ToggleButton> setListSlotArray;
    OwnedArray<TextButton> deleteBtnArray;
    OwnedArray<TextButton> selectBtnArray;

    TextButton addNewRowBtn;
    PopupMenu choosePresetMenu;

    int setListLength;
    int selectedPreset;
    const int ROW_HEIGHT, ROW_WIDTH;

    const float fontSize;
    const Rectangle<int> &panelBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel);
};

#endif  // __PRESETWINDOW_H_F0BDFA0B__
