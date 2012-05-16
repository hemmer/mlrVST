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
class mlrVSTAudioProcessor;


class PresetPanel :
    public Component,
    public ButtonListener
{

public:

    PresetPanel(const Rectangle<int> &bounds,
                mlrVSTAudioProcessor * const owner);

    void buttonClicked(Button *btn);
    void paint(Graphics& g);

private:

    // communication //////////////////////
    mlrVSTAudioProcessor * const processor;

    // gui setup //////////////////////////
    Label panelLabel;
    const float fontSize;
    const Rectangle<int> &panelBounds;

    // setlist ///////////////////////////////
    OwnedArray<ToggleButton> setListSlotArray;
    OwnedArray<TextButton> deleteBtnArray;
    OwnedArray<TextButton> selectBtnArray;

    TextButton addNewRowBtn;
    PopupMenu choosePresetMenu;

    int selectedPreset;
    int setListLength;

    const int ROW_HEIGHT, ROW_WIDTH;

    void addRow();
    void deleteRow(const int &index);
    void arrangeButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel);
};

#endif  // __PRESETWINDOW_H_F0BDFA0B__
