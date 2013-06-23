#include "PresetPanel.h"
#include "mlrVSTGUI.h"

PresetPanel::PresetPanel(const Rectangle<int> &bounds,
                         mlrVSTAudioProcessor * const owner) :
// communication //////////////////////
processor(owner),
    // gui setup //////////////////////////
    panelLabel("preset panel label", "setlist manager"),
    instructionLabel(),
    defaultFont("ProggyCleanTT", 18.f, Font::plain), panelBounds(bounds),
    loadSetlistBtn("load setlist from file", "load setlist"),
    saveSetlistBtn("save setlist to file", "save setlist"),
    presetListTbl(owner), setlistTbl(owner, &presetListTbl)
{
    int xPosition = 0;
    int yPosition = 0;

    // add the header label
    addAndMakeVisible(&panelLabel);
    panelLabel.setBounds(xPosition, yPosition, panelBounds.getWidth(), 36);
    panelLabel.setColour(Label::backgroundColourId, Colours::black);
    panelLabel.setColour(Label::textColourId, Colours::white);
    Font titleFont("ProggyCleanTT", 36.f, Font::plain);
    panelLabel.setFont(titleFont);
    yPosition += 30 + PAD_AMOUNT;
    xPosition += PAD_AMOUNT;

    // instructions for how to use the setlist
    addAndMakeVisible(&instructionLabel);
    instructionLabel.setBounds(xPosition, yPosition, 500, 60);
    instructionLabel.setColour(Label::backgroundColourId, Colours::black);
    instructionLabel.setColour(Label::textColourId, Colours::white);
    instructionLabel.setFont(defaultFont);
    instructionLabel.setText("To create a setlist, drag presets from the list on the left to the setlist panel on the right...", NotificationType::dontSendNotification);

    addAndMakeVisible(&loadSetlistBtn);
    loadSetlistBtn.addListener(this);
    loadSetlistBtn.setBounds(520, yPosition, 150, 25);

    addAndMakeVisible(&saveSetlistBtn);
    saveSetlistBtn.addListener(this);
    saveSetlistBtn.setBounds(520, yPosition + 35, 150, 25);

    yPosition += 60 + PAD_AMOUNT;

    addAndMakeVisible(&presetListTbl);
    presetListTbl.setBounds(10, yPosition, 400, 400);

    addAndMakeVisible(&setlistTbl);
    setlistTbl.setBounds(420, yPosition, 282, 400);
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
}

void PresetPanel::visibilityChanged()
{
    if (isVisible())
        refreshPresetLists();

}