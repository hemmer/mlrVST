/*
  ==============================================================================

    mlrVST

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ChannelStrip.h"


//==============================================================================
mlrVSTAudioProcessorEditor::mlrVSTAudioProcessorEditor (mlrVSTAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      infoLabel(), helloLabel("", "Hello"), logoLabel("", "mlrVST"), delayLabel("", "Delay:"),
      delaySlider("delay"), gainSlider("gain"),
	  loadButton("loadfile", DrawableButton::ImageRaw),
	  waveformArray(),
	  numChannels(4),
      numStrips(7),
	  loadedFiles(),
      debugLabel(), debugButton("loadfile", DrawableButton::ImageRaw),    // debugging stuff
      channelArray(), slidersArray()
{
    setSize(GUI_WIDTH, GUI_HEIGHT);

    // add logo strip to top
	addAndMakeVisible(&logoLabel);
    logoLabel.setBounds(0, 0, getWidth(), 30);
	logoLabel.setColour(Label::backgroundColourId, Colours::black);
    logoLabel.setColour(Label::textColourId, Colours::white);
    logoLabel.setJustificationType(Justification::bottomRight);
    logoLabel.setFont(30.0f);
	
    DBG("editor loaded");

    // DELAY stuff (may eventually go)
    addAndMakeVisible(&delaySlider);
	delaySlider.setSliderStyle(Slider::LinearVertical);
    delaySlider.addListener(this);
    delaySlider.setRange(0.0, 1.0, 0.01);
	delaySlider.setValue(0.02);
	delaySlider.setBounds(350, 80, 150, 40);
    // attach label
    delayLabel.attachToComponent(&delaySlider, false);
    delayLabel.setFont (Font (11.0f));


    // For manually loading files
    addAndMakeVisible(&loadButton);
	loadButton.setBounds(30, 50, 100, 30);
	loadButton.addListener(this);
	loadButton.setBackgroundColours(Colours::blue, Colours::black);


    // add a label that will display the current timecode and status..
    addAndMakeVisible(&infoLabel);
    infoLabel.setColour(Label::textColourId, Colours::black);
	infoLabel.setBounds(10, getHeight() - 25, 400, 25);

    // useful UI debugging components
    addAndMakeVisible(&debugLabel);
    debugLabel.setColour(Label::textColourId, Colours::black);
	debugLabel.setBounds(10, 80, 400, 25);
    addAndMakeVisible(&debugButton);
	debugButton.addListener(this);
	debugButton.setBackgroundColours(Colours::blue, Colours::black);
	debugButton.setBounds(400, 300, 50, 25);
    // another test label
	addAndMakeVisible(&helloLabel);
	helloLabel.setBounds(350, 150, 100, 100);
	helloLabel.setColour(Label::backgroundColourId, Colours::bisque);

    // manually add channels (TODO automate this)
    // possible adding ColourSets or similar
    channelArray.add(ChannelStrip(Colour(226,  70,  45), 0));
    channelArray.add(ChannelStrip(Colour(106,  22,  37), 1));
    channelArray.add(ChannelStrip(Colour( 73, 108, 104), 2));
    channelArray.add(ChannelStrip(Colour( 33,  61,  75), 3));
    channelArray.add(ChannelStrip(Colour(250, 241, 162), 4));

    // add waveform strips
    for(int i = 0; i < numStrips; ++i){
        waveformArray.add(new WaveformControl(i, channelArray));
        waveformArray[i]->setBounds(30, 80 + i * 80, 300, 75);
        addAndMakeVisible( waveformArray[i] );
	}

    /////////////////////////////
    // Add all volume controls //
    /////////////////////////////
    // Master volume
    addAndMakeVisible(&gainSlider);
    gainSlider.setSliderStyle(Slider::LinearVertical);
    gainSlider.addListener(this);
    gainSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setValue(0.05);
    gainSlider.setBounds(350, 500, 30, 150);
    gainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);

    //slidersArray.clear();
    // Programmatically add group volume controls
    for(int i = 0; i < channelArray.size(); ++i)
    {
        slidersArray.add(new Slider("channel " + String(i) + " vol"));
        addAndMakeVisible(slidersArray[i]);
        slidersArray[i]->setSliderStyle(Slider::LinearVertical);
        slidersArray[i]->addListener(this);
        slidersArray[i]->setRange(0.0, 1.0, 0.01);
        slidersArray[i]->setValue(0.8);
        slidersArray[i]->setBounds(380 + i*30, 500, 30, 150);
        slidersArray[i]->setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);
        slidersArray[i]->setColour(Slider::backgroundColourId, channelArray[i].getColour());
    }

    formatManager.registerBasicFormats();
    
    // This doesn't seem to work 
    // TODO: Make custom look and feel
    // OldSchoolLookAndFeel oldLookAndFeel;
    // LookAndFeel::setDefaultLookAndFeel(&oldLookAndFeel);
	
    startTimer (50);
}

mlrVSTAudioProcessorEditor::~mlrVSTAudioProcessorEditor()
{
    //// programatically remove volume controls
    //for(int i = 0; i < slidersArray.size(); ++i)
    //{
    //    Slider *tempSlider = slidersArray[i];
    //    deleteAndZero(tempSlider);
    //}
 
}

void mlrVSTAudioProcessorEditor::debugMe(const String str)
{
    debugLabel.setText(str, false);
}

// receieve communication from the WaveformControl component
void mlrVSTAudioProcessorEditor::recieveFileSelection(const int &waveformID, const int &fileChoice){
    helloLabel.setText(String(waveformID) + ", id: " + String(fileChoice), false);
    waveformArray[waveformID]->setFile(loadedFiles[fileChoice]);
}

// return a copy of the current filelist array
Array<File> mlrVSTAudioProcessorEditor::getLoadedFiles(){
    return Array<File>(loadedFiles);
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

    AudioPlayHead::CurrentPositionInfo newPos(ourProcessor->lastPosInfo);

    if (lastDisplayedPosition != newPos)
        displayPositionInfo (newPos);

    delaySlider.setValue(ourProcessor->delay, false);
}

// This is our Slider::Listener callback, when the user drags a slider.
void mlrVSTAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    if (slider == &gainSlider)
    {
        // It's vital to use setParameterNotifyingHost to change any parameters that are automatable
        // by the host, rather than just modifying them directly, otherwise the host won't know
        // that they've changed.
        getProcessor()->setParameterNotifyingHost (mlrVSTAudioProcessor::gainParam,
                                                   (float) gainSlider.getValue());
    }
    else if (slider == &delaySlider)
    {
        getProcessor()->setParameterNotifyingHost (mlrVSTAudioProcessor::delayParam,
                                                   (float) delaySlider.getValue());
    }
}

// Button handling
void mlrVSTAudioProcessorEditor::buttonClicked(Button* btn)
{
    // Manually load a file
	if(btn == &loadButton)
    {

		FileChooser myChooser ("Please choose a file:", File::getSpecialLocation(File::userDesktopDirectory), "*.wav");
		if(myChooser.browseForFileToOpen())
		{	
			File newFile = myChooser.getResult();
			loadedFiles.addIfNotAlreadyThere(newFile);

            for(int i = 0; i < waveformArray.size(); ++i){
				waveformArray[i]->setFile(newFile);
			}

			String str = T("Files Loaded:\n");
			for(int i = 0; i < loadedFiles.size(); ++i)
            {
                str += String(loadedFiles[i].getFileNameWithoutExtension()) + "\n";
			}
			helloLabel.setText(str, false);
		}
	}
    else if(btn == &debugButton)
    {
        // just a little test (REMEMBER TO REMOVE)
        channelArray.clear();
        channelArray.add(ChannelStrip(Colour(226,  70,  45), 0));
        channelArray.add(ChannelStrip(Colour(106,  22,  37), 1));
        channelArray.add(ChannelStrip(Colour( 73, 108, 104), 2));

        for(int i = 0; i < waveformArray.size(); ++i)
        {
            waveformArray[i]->updateChannelList(channelArray);
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

    infoLabel.setText(displayText, false);
}
