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
#include "WaveformControl.h"

class WaveformControl;

//==============================================================================
/** This is the editor component that our filter will display.
*/
class mlrVSTAudioProcessorEditor  : public AudioProcessorEditor,
                                            public SliderListener,
											public ButtonListener,
                                            public Timer
{
public:
    mlrVSTAudioProcessorEditor (mlrVSTAudioProcessor* ownerFilter);
    ~mlrVSTAudioProcessorEditor();

    //==============================================================================
    void timerCallback();
    void paint (Graphics& g);
    void sliderValueChanged (Slider*);
	void buttonClicked(Button*);

private:
    MidiKeyboardComponent midiKeyboard;
    Label infoLabel, delayLabel, helloLabel;
    Slider delaySlider;
	DrawableButton loadButton;

	Array<File> loadedFiles;

	AudioFormatManager formatManager;
    //AudioThumbnailCache thumbnailCache;
    //AudioThumbnail thumbnail;
	//WaveformControl *testWaveform;

	void loadFileIntoTransport (const File& audioFile);
	AudioTransportSource transportSource;
    ScopedPointer<AudioFormatReaderSource> currentAudioFileSource;

	Array<WaveformControl*> waveformArray;
	const int numChannels;

    AudioPlayHead::CurrentPositionInfo lastDisplayedPosition;

    mlrVSTAudioProcessor* getProcessor() const
    {
        return static_cast <mlrVSTAudioProcessor*> (getAudioProcessor());
    }

    void displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos);
};


#endif  // __PLUGINEDITOR_H_4ACCBAA__
