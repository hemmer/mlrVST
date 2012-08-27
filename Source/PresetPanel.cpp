

#include "PresetPanel.h"
#include "PluginEditor.h"


PresetPanel::PresetPanel(const Rectangle<int> &bounds,
                         mlrVSTAudioProcessor * const owner) :
    // communication //////////////////////
    processor(owner),
    // gui setup //////////////////////////
    panelLabel("preset panel label", "Setlist Manager"),
    fontSize(7.4f), panelBounds(bounds),
    loadSetlistBtn("load setlist", "load setlist"),
    saveSetlistBtn("save setlist", "save setlist"),
    // setlist ///////////////////////////////
    setListSlotArray(), deleteBtnArray(), selectBtnArray(),
    addNewRowBtn("Add new"), choosePresetMenu(),
    selectedPreset(0), setListLength(0),
    ROW_HEIGHT(20), ROW_WIDTH(250)
{
    addAndMakeVisible(&panelLabel);
    panelLabel.setBounds(0, 0, panelBounds.getWidth(), 30);
    panelLabel.setColour(Label::backgroundColourId, Colours::black);
    panelLabel.setColour(Label::textColourId, Colours::white);
    panelLabel.setFont(2.0f * fontSize);

    addAndMakeVisible(&loadSetlistBtn);
    loadSetlistBtn.addListener(this);
    loadSetlistBtn.setBounds(400, 350, 70, 25);
    addAndMakeVisible(&saveSetlistBtn);
    saveSetlistBtn.addListener(this);
    saveSetlistBtn.setBounds(500, 350, 70, 25);

    addAndMakeVisible(&addNewRowBtn);
    addNewRowBtn.addListener(this);

    // set up the initial setlist state
    arrangeButtons();
}

void PresetPanel::paint(Graphics &g)
{
    g.fillAll(Colours::grey.withAlpha(0.9f));
}

void PresetPanel::buttonClicked(Button *btn)
{
    // load setlist manually using file dialog
    // TODO: drop-n-drag for setlist?
    if(btn == &loadSetlistBtn)
    {
        FileChooser myChooser ("Please choose a setlist to load:", File::getSpecialLocation(File::userDesktopDirectory), "*.xml");

        // popup dialog to open setlist
        if(myChooser.browseForFileToOpen())
        {	
            // if sucessful, try load this setlist
            File newSetlist = myChooser.getResult();
            processor->loadXmlSetlist(newSetlist);
        }
    }

    else if(btn == &saveSetlistBtn)
    {
        FileChooser myChooser ("Save this setlist:", File::getSpecialLocation(File::userDesktopDirectory), "*.xml");

        // popup dialog to open setlist
        if(myChooser.browseForFileToSave(true))
        {	
            // if sucessful, try save this setlist
            File newSetlist = myChooser.getResult();
            processor->saveXmlSetlist(newSetlist);
        }
    }

    else if (btn == &addNewRowBtn) addRow();
    else
    {
        for (int b = 0; b < setListSlotArray.size(); ++b)
        {
            if (deleteBtnArray[b] == btn)
            {
                deleteRow(b);
            }
            else if (selectBtnArray[b] == btn)
            {
                processor->switchPreset(b);
            }
            else if (setListSlotArray[b] == btn)
            {

                XmlElement currentPresetList = processor->getPresetList();
                XmlElement currentSetlist = processor->getSetlist();

                // The button clicked corresponds to this preset slot
                XmlElement* presetToChange = currentSetlist.getChildElement(b);

                // Populate the popup menu with all possible presets
                PopupMenu presetSelectMenu = PopupMenu();
                int index = 1;
                presetSelectMenu.addItem(index, "None");

                const String nameAttribute = processor->getGlobalSettingName(mlrVSTAudioProcessor::sPresetName);
                forEachXmlChildElement(currentPresetList, p)
                {
                    ++index;
                    const String pName = p->getStringAttribute(nameAttribute);
                    presetSelectMenu.addItem(index, pName);
                }


                int presetChoice = presetSelectMenu.showMenu(PopupMenu::Options().withTargetComponent(setListSlotArray[b]));

                // If "none" is selected, insert a blank preset
                if (presetChoice == 1)
                {
                    String presetChoiceName = "#" + String(b) + ": None";
                    setListSlotArray[b]->setButtonText(presetChoiceName);
                    currentSetlist.replaceChildElement(presetToChange, new XmlElement("PRESETNONE"));
                }
                // otherwise insert the choosen preset
                else if (presetChoice > 1)
                {
                    XmlElement chosenPreset = *(currentPresetList.getChildElement(presetChoice - 2));
                    currentSetlist.replaceChildElement(presetToChange, new XmlElement(chosenPreset));

                    String presetChoiceName = "#" + String(b) + ": " + chosenPreset.getStringAttribute(nameAttribute);
                    setListSlotArray[b]->setButtonText(presetChoiceName);
                }

                setListSlotArray[b]->setToggleState(false, false);
                processor->setSetlist(currentSetlist);
            }

        }
    }
}

void PresetPanel::addRow()
{
    XmlElement currentSetlist = processor->getSetlist();

    // add a new blank preset
    currentSetlist.createNewChildElement("PRESETNONE");
    // let the processor know about the change
    processor->setSetlist(currentSetlist);

    // and rebuild setlist manager
    arrangeButtons();
}

void PresetPanel::deleteRow(const int &index)
{
    XmlElement currentSetlist = processor->getSetlist();
    XmlElement* itemToRemove = currentSetlist.getChildElement(index);

    // add a new blank preset
    currentSetlist.removeChildElement(itemToRemove, true);
    // let the processor know about the change
    processor->setSetlist(currentSetlist);

    // and rebuild setlist manager
    arrangeButtons();
}

void PresetPanel::arrangeButtons()
{
    // get the current setlist
    XmlElement currentSetlist = processor->getSetlist();

    // TODO: rebuilding each time is maybe a little inefficient
    setListSlotArray.clear(true);
    deleteBtnArray.clear(true);
    selectBtnArray.clear(true);

    int index = 0;
    forEachXmlChildElement(currentSetlist, child)
    {
        String presetName = child->getStringAttribute("name");
        String buttonLblText = "#" + String(index) + ": " + presetName;

        // these buttons are clicked to select which preset is in the slot
        setListSlotArray.add(new ToggleButton(buttonLblText));
        addAndMakeVisible(setListSlotArray.getLast());
        setListSlotArray.getLast()->addListener(this);
        setListSlotArray.getLast()->setBounds(PAD_AMOUNT,
            50 + index * (ROW_HEIGHT + PAD_AMOUNT), ROW_WIDTH, ROW_HEIGHT);

        // load the selected item in the preset list
        selectBtnArray.add(new TextButton("SEL"));
        addAndMakeVisible(selectBtnArray.getLast());
        selectBtnArray.getLast()->addListener(this);
        selectBtnArray.getLast()->setBounds(ROW_WIDTH,
            50 + index * (ROW_HEIGHT + PAD_AMOUNT), 25, ROW_HEIGHT);
        selectBtnArray.getLast()->setColour(TextButton::textColourOnId, Colours::grey);

        // deletes the current item from the preset list
        deleteBtnArray.add(new TextButton("DEL"));
        addAndMakeVisible(deleteBtnArray.getLast());
        deleteBtnArray.getLast()->addListener(this);
        deleteBtnArray.getLast()->setBounds(ROW_WIDTH + 3*PAD_AMOUNT,
            50 + index * (ROW_HEIGHT + PAD_AMOUNT), 25, ROW_HEIGHT);

        // iterate away
        ++index;
    }

    // Finally add the option to add a new row
    addNewRowBtn.setBounds(PAD_AMOUNT, 50 + index * (ROW_HEIGHT + PAD_AMOUNT),
        50, ROW_HEIGHT);
}

