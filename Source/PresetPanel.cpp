

#include "PresetPanel.h"
#include "PluginEditor.h"


PresetPanel::PresetPanel(const Rectangle<int> &bounds,
                         mlrVSTAudioProcessorEditor * const owner) :
    mlrVSTEditor(owner),
    panelLabel("preset panel label", "Setlist Manager"),
    fontSize(7.4f), panelBounds(bounds),
    setListLength(0), ROW_HEIGHT(20), ROW_WIDTH(280), PAD_AMOUNT(10),
    setListSlotArray(), deleteBtnArray(),
    addNewRowBtn("Add new")
{
    addAndMakeVisible(&panelLabel);
    panelLabel.setBounds(0, 0, panelBounds.getWidth(), 30);
    panelLabel.setColour(Label::backgroundColourId, Colours::black);
    panelLabel.setColour(Label::textColourId, Colours::white);
    panelLabel.setFont(2 * fontSize);

    addAndMakeVisible(&addNewRowBtn);
    addNewRowBtn.addListener(this);

    // set up the initial setlist state
    arrangeButtons();
}

void PresetPanel::paint(Graphics &g)
{
    g.fillAll(Colours::black.withAlpha(0.4f));
}

void PresetPanel::buttonClicked(Button *btn)
{

    if (btn == &addNewRowBtn) addRow();

    for (int b = 0; b < setListSlotArray.size(); ++b)
    {
        if (deleteBtnArray[b] == btn)
        {
            deleteRow(b);
        }
        else if (setListSlotArray[b] == btn)
        {

            XmlElement currentPresetList = mlrVSTEditor->getPresetList();
            XmlElement currentSetlist = mlrVSTEditor->getSetlist();

            // The button clicked corresponds to this preset slot
            XmlElement* presetToChange = currentSetlist.getChildElement(b);

            // Populate the popup menu with all possible presets
            PopupMenu presetSelectMenu = PopupMenu();
            int index = 1;
            presetSelectMenu.addItem(index, "None");

            XmlElement* p = currentPresetList.getFirstChildElement();
            while (p != nullptr)
            {
                ++index;
                String pName = p->getStringAttribute("name");
                presetSelectMenu.addItem(index, pName);
                p = p->getNextElement();
            }


            int presetChoice = presetSelectMenu.showMenu(PopupMenu::Options().withTargetComponent(setListSlotArray[b]));
            
            // If "none" is selected, insert a blank preset
            if (presetChoice == 1)
            {
                String presetChoiceName = "#" + String(b) + ": None";
                setListSlotArray[b]->setButtonText(presetChoiceName);
                currentSetlist.replaceChildElement(presetToChange, new XmlElement("PRESET_NONE"));
            }
            // otherwise insert the choosen preset
            else if (presetChoice > 1)
            {
                XmlElement chosenPreset = *(currentPresetList.getChildElement(presetChoice - 2));
                currentSetlist.replaceChildElement(presetToChange, new XmlElement(chosenPreset));

                String presetChoiceName = "#" + String(b) + ": " + chosenPreset.getStringAttribute("name");
                setListSlotArray[b]->setButtonText(presetChoiceName);
            }

            setListSlotArray[b]->setToggleState(false, false);
            mlrVSTEditor->setSetlist(currentSetlist);
        }

    }
}

void PresetPanel::addRow()
{
    XmlElement currentSetlist = mlrVSTEditor->getSetlist();
    
    // add a new blank preset
    currentSetlist.createNewChildElement("PRESET_NONE");
    // let the processor know about the change
    mlrVSTEditor->setSetlist(currentSetlist);

    // and rebuild setlist manager
    arrangeButtons();
}

void PresetPanel::deleteRow(const int &index)
{
    XmlElement currentSetlist = mlrVSTEditor->getSetlist();
    XmlElement* itemToRemove = currentSetlist.getChildElement(index);

    // add a new blank preset
    currentSetlist.removeChildElement(itemToRemove, true);
    // let the processor know about the change
    mlrVSTEditor->setSetlist(currentSetlist);

    // and rebuild setlist manager
    arrangeButtons();
}

void PresetPanel::arrangeButtons()
{
    // get the current setlist
    XmlElement currentSetlist = mlrVSTEditor->getSetlist();

    // TODO: rebuilding each time is maybe a little inefficient
    setListSlotArray.clear(true);
    deleteBtnArray.clear(true);

    XmlElement* child = currentSetlist.getFirstChildElement();
    int index = 0;
    while (child != nullptr)
    {
        String presetName = child->getStringAttribute("name");
        String buttonLblText = "#" + String(index) + ": " + presetName;

        setListSlotArray.add(new ToggleButton(buttonLblText));
        addAndMakeVisible(setListSlotArray.getLast());
        setListSlotArray.getLast()->addListener(this);
        setListSlotArray.getLast()->setBounds(PAD_AMOUNT,
            50 + index * (ROW_HEIGHT + PAD_AMOUNT), ROW_WIDTH, ROW_HEIGHT);

        deleteBtnArray.add(new TextButton("DEL"));
        addAndMakeVisible(deleteBtnArray.getLast());
        deleteBtnArray.getLast()->addListener(this);
        deleteBtnArray.getLast()->setBounds(ROW_WIDTH + 2*PAD_AMOUNT,
            50 + index * (ROW_HEIGHT + PAD_AMOUNT), 25, ROW_HEIGHT);

        // iterate away
        child = child->getNextElement();
        ++index;
    }

    // Finally add the option to add a new row
    addNewRowBtn.setBounds(PAD_AMOUNT, 50 + index * (ROW_HEIGHT + PAD_AMOUNT),
        50, ROW_HEIGHT);
}
