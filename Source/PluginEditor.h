/*
  ==============================================================================

    This file was auto-generated by the Jucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef __PLUGINEDITOR_H_4ACCBAA__
#define __PLUGINEDITOR_H_4ACCBAA__



#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/JucePluginCharacteristics.h"
#include "PluginProcessor.h"
#include "SampleStripControl.h"
#include "mlrVSTLookAndFeel.h"


class WaveformControl;

//==============================================================================
/** This is the editor component that our filter will display.
*/
class mlrVSTAudioProcessorEditor  : public AudioProcessorEditor,
                                    public SliderListener,
									public ButtonListener,
                                    public ComboBoxListener,
                                    public Timer, 
                                    public FileDragAndDropTarget
{
public:
    mlrVSTAudioProcessorEditor (mlrVSTAudioProcessor* ownerFilter, const int &newNumChannels);
    ~mlrVSTAudioProcessorEditor();

    //==============================================================================
    void timerCallback();
    void paint(Graphics& g);

    // These are required so that the WaveformControls
    // can handle sample loading by Drag n' Drop 
    bool isInterestedInFileDrag(const StringArray&) { return true; }
    void filesDropped(const StringArray&, int, int) { }


    // listeners
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void sliderValueChanged (Slider*);
	void buttonClicked(Button*);

    /* These just forward the various requests for sample information
       to the AudioProcessor (which holds the sample pool). */
    int getSamplePoolSize() { return getProcessor()->getSamplePoolSize(); }
    String getSampleName(const int &index) { return getProcessor()->getSampleName(index); }
    // AudioSample* getSample(const int &index) { return getProcessor()->getSample(index); }
    // AudioSample* getLatestSample() { return getProcessor()->getLatestSample(); }
    // this returns false if the file failed to load, true otherwise
    bool loadSampleFromFile(File &sampleFile) { return getProcessor()->addNewSample(sampleFile); }
    
    // Pass SampleStripControl messages back to the plugin processor
    File getSampleSourceFile(const int &index) const { return getProcessor()->getSample(index)->getSampleFile(); }



    // Forwards get / set requests for SampleStrips to the processor
    void setSampleStripParameter(const int &parameterID, const void *newValue, const int &stripID)
    {
        getProcessor()->setSampleStripParameter(parameterID, newValue, stripID);
    }
    const void * getSampleStripParameter(const int &parameterID, const int &stripID)
    {
        return getProcessor()->getSampleStripParameter(parameterID, stripID);
    }

    void calcInitialPlaySpeed(const int &stripID) { getProcessor()->calcInitialPlaySpeed(stripID); }
    void updatePlaySpeedForNewSelection(const int &stripID) { getProcessor()->calcPlaySpeedForSelectionChange(stripID); }
    void modPlaySpeed(const double &factor, const int &stripID) { getProcessor()->modPlaySpeed(factor, stripID); }
    AudioSample * getAudioSample(const int &poolIndex) const { return getProcessor()->getAudioSample(poolIndex); }

    Colour getChannelColour(const int &chan) const { return getProcessor()->getChannelProcessor(chan)->getChannelColour(); }

private:
    Label infoLabel, helloLabel, bpmLabel;
    Slider masterGainSlider;
	DrawableButton debugButton;
    ComboBox selNumChannels;
	ListBox fileList;

    float fontSize;

	AudioFormatManager formatManager;

	// Store the waveform controls/strips in array. 
    // For a standard monome64 this is 7
    OwnedArray<SampleStripControl> sampleStripControlArray;
    const int numStrips;
    const int waveformControlHeight, waveformControlWidth;

    // This is the number of seperate channels. In 
    // pratical terms, this is just the number of 
    // samples that can be played at once.
	int numChannels;

    OwnedArray<Slider> slidersArray;
    void buildSliders();

    // For simplicity, let's stick to a fixed size GUI
    static const int GUI_HEIGHT = 750;
    static const int GUI_WIDTH = 1024;
    static const int PAD_AMOUNT = 10;
    AudioPlayHead::CurrentPositionInfo lastDisplayedPosition;

    mlrVSTAudioProcessor* getProcessor() const { return static_cast <mlrVSTAudioProcessor*> (getAudioProcessor()); }

    void displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos);

    mlrVSTLookAndFeel myLookAndFeel;

    JUCE_LEAK_DETECTOR(mlrVSTAudioProcessorEditor);  
};


#endif  // __PLUGINEDITOR_H_4ACCBAA__
