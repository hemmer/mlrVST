#ifndef __PLUGINEDITOR_H_4ACCBAA__
#define __PLUGINEDITOR_H_4ACCBAA__



#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "SampleStripControl.h"
#include "PresetPanel.h"
#include "SettingsPanel.h"
#include "MappingPanel.h"
#include "timedButton.h"
#include "mlrVSTLookAndFeel.h"
#include "PatternStripControl.h"
#include "HintOverlay.h"


#ifndef PAD_AMOUNT
#define PAD_AMOUNT 10
#endif

#ifndef GUI_WIDTH
#define GUI_WIDTH 1024
#endif

#ifndef GUI_HEIGHT
#define GUI_HEIGHT 650
#endif


class WaveformControl;
class PatternStripControl;


class mlrVSTGUI  : public AudioProcessorEditor,
                                    public SliderListener,
									public ButtonListener,
                                    public ComboBoxListener,
                                    public ChangeListener,
                                    public Timer,
                                    public FileDragAndDropTarget
{
public:
    mlrVSTGUI (mlrVSTAudioProcessor* ownerFilter,
                                const int &newNumChannels,
                                const int &newNumStrips);
    ~mlrVSTGUI();


	enum { modeSampleStrips, modePatternStrips };


    //==============================================================================
    void timerCallback();
    void paint(Graphics& g);

    // These are required so that the WaveformControls
    // can handle sample loading by Drag n' Drop
    bool isInterestedInFileDrag(const StringArray&) { return true; }
    void filesDropped(const StringArray&, int, int) { }

    // for receiving messages when settings are changed
    void changeListenerCallback(ChangeBroadcaster *);

    // listeners for gui components
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void sliderValueChanged (Slider*);
	void buttonClicked(Button*);

    /* These just forward the various requests for sample information
       to the AudioProcessor (which holds the sample pool). */
    int getSamplePoolSize(const int &poolID) { return parent->getSamplePoolSize(poolID); }
    String getSampleName(const int &index, const int &poolID) { return parent->getSampleName(index, poolID); }

    void switchChannels(const int &newChan, const int &stripID) const { parent->switchChannels(newChan, stripID); }
    Colour getChannelColour(const int &chan) const { return parent->getChannelColour(chan); }

    void updateGlobalSetting(const int &parameterID, const void *newValue);
    const void* getGlobalSetting(const int &parameterID) const { return parent->getGlobalSetting(parameterID); }

    void buildSampleStripControls(const int &newNumStrips);

private:

    // Communication ///////////////////
    mlrVSTAudioProcessor * const parent;

    // Style / positioning objects ///////////////
    mlrVSTLookAndFeel myLookAndFeel;
	overrideLookandFeel overrideLF;

    // these are just helpers for positioning
    int xPosition, yPosition;

    // Fonts ///////////////////////
    float fontSize; Font defaultFont;

    // Volume controls //////////////////////////////
    Slider masterGainSlider; Label masterSliderLabel;
    OwnedArray<Slider> slidersArray;
    OwnedArray<ToggleButton> slidersMuteBtnArray;
    // this just sets it all up
    void buildSliders();

    // Tempo controls /////////////////
    Slider bpmSlider; Label bpmLabel;                       // bpm components
    ComboBox quantiseSettingsCbox; Label quantiseLabel;     // quantise options components
    void setupTempoUI();


    // Buttons ////////////////////////////
    TextButton loadFilesBtn;
	DrawableButton sampleStripToggle, patternStripToggle;
	DrawableImage sampleImg, patternImg;

    // Record / Resample / Pattern UI ////////////////////////////////
    Label precountLbl, recordLengthLbl, bankLbl;
    // sliders for choosing banks etc
    Slider recordPrecountSldr, recordLengthSldr, recordBankSldr;
    Slider resamplePrecountSldr, resampleLengthSldr, resampleBankSldr;
    Slider patternPrecountSldr, patternLengthSldr, patternBankSldr;
    // these give visual progress of recording / resampling status
    TimedButton recordBtn, resampleBtn, patternBtn;
    void setUpRecordResampleUI();

	Label vstNameLbl;

    // Misc /////////////////////
    // this object is used to store bpm information from the host
    AudioPlayHead::CurrentPositionInfo lastDisplayedPosition;
    DrawableButton debugButton;

    // Panels ///////////////////////
    void setupPanels();     // positions panels / buttons
    void closePanels();     // closes all panels

    //// Presets //////////////////////
	Label presetLabel;
    TextButton addPresetBtn;
    ToggleButton toggleSetlistBtn;
    Rectangle<int> presetPanelBounds;
    PresetPanel presetPanel;
    ComboBox presetCbox;


    //// Settings ///////////////
    int numChannels;        // sets number of simultaneous audio channels
    bool useExternalTempo;  // are we using the bpm from the host?
    // These provide the overlay for the settings Panel
    ToggleButton toggleSettingsBtn;
    Rectangle<int> settingsPanelBounds;
    SettingsPanel settingsPanel;

    //// Mappings ///////////////////
    Rectangle<int> mappingPanelBounds;
    ToggleButton toggleMappingBtn;
    MappingPanel mappingPanel;



    // SampleStrip controls ///////////////////////////////
    // Store the waveform controls/strips in array, this is
    // NUM_ROWS - 1 (for the top row controls)
    OwnedArray<SampleStripControl> sampleStripControlArray;
    int numStrips;
    int waveformControlHeight, waveformControlWidth;

	OwnedArray<PatternStripControl> patternStripArray;
	void setupPatternOverlays();

	
	int displayMode;		// are we showing Sample


	HintOverlay hintOverlay;

    JUCE_LEAK_DETECTOR(mlrVSTGUI);
};


#endif  // __PLUGINEDITOR_H_4ACCBAA__
