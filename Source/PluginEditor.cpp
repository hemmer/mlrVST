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
      infoLabel(0),
	  helloLabel(0),
      logoLabel(0),
      delayLabel(0),
      delaySlider(0),
	  loadButton(0),
	  waveformArray(),
	  numChannels(4),
	  loadedFiles()
{
    setSize(GUI_WIDTH, GUI_HEIGHT);

    // test label
	addAndMakeVisible(logoLabel = new Label("", "mlrVST"));
    logoLabel->setBounds(0, 0, getWidth(), 20);
	logoLabel->setColour(Label::backgroundColourId, Colours::black);
    logoLabel->setColour(Label::textColourId, Colours::white);
    logoLabel->setJustificationType(Justification::bottomRight);
	
	// test label
	addAndMakeVisible(helloLabel = new Label("", "Hello"));
	helloLabel->setBounds(350, 150, 100, 100);
	helloLabel->setColour(Label::backgroundColourId, Colours::bisque);

    addAndMakeVisible (delaySlider = new Slider("delay"));
	delaySlider->setSliderStyle (Slider::LinearHorizontal);
    delaySlider->addListener (this);
    delaySlider->setRange (0.0, 1.0, 0.01);
	delaySlider->setValue (0.02);
	delaySlider->setBounds (350, 80, 150, 40);

    addAndMakeVisible(loadButton = new DrawableButton("loadfile", DrawableButton::ImageRaw));
	loadButton->setBounds(30, 50, 100, 30);
	loadButton->addListener (this);
	loadButton->setBackgroundColours(Colours::red, Colours::black);

	formatManager.registerBasicFormats();


    delayLabel = new Label("", "Delay:");
    delayLabel->attachToComponent(delaySlider, false);
    delayLabel->setFont (Font (11.0f));


    // add a label that will display the current timecode and status..
    addAndMakeVisible(infoLabel = new Label());
    infoLabel->setColour (Label::textColourId, Colours::black);
	infoLabel->setBounds (10, getHeight() - 25, 400, 25);

	Array<Colour> bgCols = Array<Colour>();
	bgCols.add(Colour(226,70,45));
    bgCols.add(Colour(106,22,37));
    bgCols.add(Colour(73,108,104));
    bgCols.add(Colour(33,61,75));
	
	for(int i = 0; i < numChannels; ++i){
		waveformArray.add(new WaveformControl(bgCols[i]));
		addAndMakeVisible ( waveformArray[i] );
		waveformArray[i]->setBounds(30, 80 + i * 50, 300, 50);
	}


	
    startTimer (50);
}

mlrVSTAudioProcessorEditor::~mlrVSTAudioProcessorEditor()
{
    deleteAndZero(infoLabel);
    deleteAndZero(delayLabel);
    deleteAndZero(helloLabel);
    deleteAndZero(logoLabel);
    deleteAndZero(delaySlider);
    deleteAndZero(loadButton);
        
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

    delaySlider->setValue (ourProcessor->delay, false);
}

// This is our Slider::Listener callback, when the user drags a slider.
void mlrVSTAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    if (slider == delaySlider)
    {
        getProcessor()->setParameterNotifyingHost (mlrVSTAudioProcessor::delayParam,
                                                   (float) delaySlider->getValue());
    }
}

void mlrVSTAudioProcessorEditor::buttonClicked(Button* btn)
{
	if(btn == loadButton){

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
			helloLabel->setText(str, false);
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

    infoLabel->setText (displayText, false);
}
