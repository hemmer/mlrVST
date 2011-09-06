/*
  ==============================================================================

    mlrVST

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
mlrVSTAudioProcessorEditor::mlrVSTAudioProcessorEditor (mlrVSTAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      midiKeyboard (ownerFilter->keyboardState, MidiKeyboardComponent::horizontalKeyboard),
      infoLabel (String::empty),
	  helloLabel ("", "Hello"),
      delayLabel ("", "Delay:"),
      delaySlider ("delay"),
	  loadButton ("loadfile", DrawableButton::ImageRaw),
	  //thumbnailTest(0),
	  waveformArray(),
	  numChannels(3),
	  loadedFiles()
{

	// test label
	addAndMakeVisible(&helloLabel);
	helloLabel.setBounds(350, 150, 100, 100);
	helloLabel.setColour(Label::backgroundColourId, Colours::bisque);

    addAndMakeVisible (&delaySlider);
    delaySlider.setSliderStyle (Slider::Rotary);
    delaySlider.addListener (this);
    delaySlider.setRange (0.0, 1.0, 0.01);
	delaySlider.setValue (0.02);
	delaySlider.setBounds (300, 60, 150, 40);

	addAndMakeVisible(&loadButton);
	loadButton.setBounds(20, 20, 100, 30);
	loadButton.addListener (this);
	loadButton.setBackgroundColours(Colours::red, Colours::black);

	formatManager.registerBasicFormats();



    delayLabel.attachToComponent (&delaySlider, false);
    delayLabel.setFont (Font (11.0f));


    // add a label that will display the current timecode and status..
    addAndMakeVisible (&infoLabel);
    infoLabel.setColour (Label::textColourId, Colours::blue);
	infoLabel.setBounds (10, 4, 400, 25);

	Array<Colour> bgCols = Array<Colour>();
	bgCols.add(Colours::red);
	bgCols.add(Colours::blue);
	bgCols.add(Colours::green);

	for(int i = 0; i < numChannels; ++i){
		waveformArray.add(new WaveformControl(bgCols[i]));
		addAndMakeVisible ( waveformArray[i] );
		waveformArray[i]->setBounds(30, 80 + i * 50, 300, 50);
	}

    setSize (500, 300);
    startTimer (50);
}

mlrVSTAudioProcessorEditor::~mlrVSTAudioProcessorEditor()
{
}

//==============================================================================
void mlrVSTAudioProcessorEditor::paint (Graphics& g)
{
    //g.setGradientFill (ColourGradient (Colours::white, 0, 0, Colours::red, 0, (float) getHeight(), false));
	g.fillAll(Colours::white);

}


//==============================================================================
// This timer periodically checks whether any of the filter's parameters have changed...
void mlrVSTAudioProcessorEditor::timerCallback()
{
    mlrVSTAudioProcessor* ourProcessor = getProcessor();

    AudioPlayHead::CurrentPositionInfo newPos (ourProcessor->lastPosInfo);

    if (lastDisplayedPosition != newPos)
        displayPositionInfo (newPos);

    delaySlider.setValue (ourProcessor->delay, false);
}

// This is our Slider::Listener callback, when the user drags a slider.
void mlrVSTAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    if (slider == &delaySlider)
    {
        getProcessor()->setParameterNotifyingHost (mlrVSTAudioProcessor::delayParam,
                                                   (float) delaySlider.getValue());
    }
}

void mlrVSTAudioProcessorEditor::buttonClicked(Button* btn)
{
	if(btn == &loadButton){

		FileChooser myChooser ("Please choose a file:", File::getSpecialLocation(File::userDesktopDirectory), "*.wav");
		if(myChooser.browseForFileToOpen())
		{	
			File newFile = myChooser.getResult();
			loadedFiles.addIfNotAlreadyThere(newFile);
			//loadFileIntoTransport(testFile);
			//testWaveform->setFile(testFile);
			for(int i = 0; i < numChannels; ++i){
				waveformArray[i]->setFile(newFile);
			}

			String str = T("Files Loaded:\n");
			for(int i = 0; i < loadedFiles.size(); ++i){
				str += String(loadedFiles[i].getFileNameWithoutExtension()) + "\n";
			}
			helloLabel.setText(str, false);
		}
	}
}

static const String timeToTimecodeString (const double seconds)
{
    const double absSecs = fabs (seconds);

    const int hours = (int) (absSecs / (60.0 * 60.0));
    const int mins  = ((int) (absSecs / 60.0)) % 60;
    const int secs  = ((int) absSecs) % 60;

    String s;
    if (seconds < 0)
        s = "-";

    s << String (hours).paddedLeft ('0', 2) << ":"
      << String (mins).paddedLeft ('0', 2) << ":"
      << String (secs).paddedLeft ('0', 2) << ":"
      << String (roundToInt (absSecs * 1000) % 1000).paddedLeft ('0', 3);

    return s;
}

// quick-and-dirty function to format a bars/beats string
static const String ppqToBarsBeatsString (double ppq, double /*lastBarPPQ*/, int numerator, int denominator)
{
    if (numerator == 0 || denominator == 0)
        return "1|1|0";

    const int ppqPerBar = (numerator * 4 / denominator);
    const double beats  = (fmod (ppq, ppqPerBar) / ppqPerBar) * numerator;

    const int bar    = ((int) ppq) / ppqPerBar + 1;
    const int beat   = ((int) beats) + 1;
    const int ticks  = ((int) (fmod (beats, 1.0) * 960.0));

    String s;
    s << bar << '|' << beat << '|' << ticks;
    return s;
}

// Updates the text in our position label.
void mlrVSTAudioProcessorEditor::displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos)
{
    lastDisplayedPosition = pos;
    String displayText;
    displayText.preallocateBytes (128);

    displayText << String (pos.bpm, 2) << " bpm, "
                << pos.timeSigNumerator << '/' << pos.timeSigDenominator
                << "  -  " << timeToTimecodeString (pos.timeInSeconds)
                << "  -  " << ppqToBarsBeatsString (pos.ppqPosition, pos.ppqPositionOfLastBarStart,
                                                    pos.timeSigNumerator, pos.timeSigDenominator);

    if (pos.isRecording)
        displayText << "  (recording)";
    else if (pos.isPlaying)
        displayText << "  (playing)";

    infoLabel.setText (displayText, false);
}
