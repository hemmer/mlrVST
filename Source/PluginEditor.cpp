/*
  ==============================================================================

    mlrVST

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

class DemoThumbnailComp  : public Component,
                           public ChangeListener,
                           public FileDragAndDropTarget
{
public:
    DemoThumbnailComp()
        : thumbnailCache (5),
          thumbnail (512, formatManager, thumbnailCache)
    {
        startTime = endTime = 0;
        formatManager.registerBasicFormats();
        thumbnail.addChangeListener (this);
    }

    ~DemoThumbnailComp()
    {
        thumbnail.removeChangeListener (this);
    }

    void setFile (const File& file)
    {
        thumbnail.setSource (new FileInputSource (file));
        startTime = 0;
        endTime = thumbnail.getTotalLength();
    }

    void setZoomFactor (double amount)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            double timeDisplayed = jmax (0.001, (thumbnail.getTotalLength() - startTime) * (1.0 - jlimit (0.0, 1.0, amount)));
            endTime = startTime + timeDisplayed;
            repaint();
        }
    }

    void mouseWheelMove (const MouseEvent&, float wheelIncrementX, float wheelIncrementY)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            double newStart = startTime + (wheelIncrementX + wheelIncrementY) * (endTime - startTime) / 10.0;
            newStart = jlimit (0.0, thumbnail.getTotalLength() - (endTime - startTime), newStart);
            endTime = newStart + (endTime - startTime);
            startTime = newStart;
            repaint();
        }
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::green);

        g.setColour (Colours::white);

        if (thumbnail.getTotalLength() > 0)
        {
            thumbnail.drawChannels (g, getLocalBounds().reduced (2, 2),
                                    startTime, endTime, 1.0f);
        }
        else
        {
            g.setFont (14.0f);
            g.drawFittedText ("(No audio file selected)", 0, 0, getWidth(), getHeight(),
                              Justification::centred, 2);
        }
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    bool isInterestedInFileDrag (const StringArray& /*files*/)
    {
        return true;
    }

    void filesDropped (const StringArray& files, int /*x*/, int /*y*/)
    {
        //AudioDemoPlaybackPage* demoPage = findParentComponentOfClass ((AudioDemoPlaybackPage*) 0);

        //if (demoPage != 0)
          //  demoPage->showFile (File (files[0]));
    }

    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    double startTime, endTime;
};



//==============================================================================
JuceDemoPluginAudioProcessorEditor::JuceDemoPluginAudioProcessorEditor (JuceDemoPluginAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      midiKeyboard (ownerFilter->keyboardState, MidiKeyboardComponent::horizontalKeyboard),
      infoLabel (String::empty),
	  helloLabel ("", "Hello"),
      delayLabel ("", "Delay:"),
      delaySlider ("delay"),
	  loadButton ("loadfile", DrawableButton::ImageRaw),
	  thumbnailTest(0)
	  //testWaveform(0)
	  //thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
{

	// test label
	addAndMakeVisible(&helloLabel);
	helloLabel.setBounds(50, 50, 100, 100);

    addAndMakeVisible (&delaySlider);
    delaySlider.setSliderStyle (Slider::Rotary);
    delaySlider.addListener (this);
    delaySlider.setRange (0.0, 1.0, 0.01);
	delaySlider.setValue (0.1);
	delaySlider.setBounds (400, 60, 150, 40);

	addAndMakeVisible(&loadButton);
	loadButton.setBounds(20, 20, 100, 30);
	loadButton.addListener (this);
	loadButton.setBackgroundColours(Colours::red, Colours::black);

	formatManager.registerBasicFormats();



    delayLabel.attachToComponent (&delaySlider, false);
    delayLabel.setFont (Font (11.0f));

    // add the midi keyboard component..
    //addAndMakeVisible (&midiKeyboard);
	//const int keyboardHeight = 70;
    //midiKeyboard.setBounds (4, 230 - 4, 500 - 8, keyboardHeight);

    // add a label that will display the current timecode and status..
    addAndMakeVisible (&infoLabel);
    infoLabel.setColour (Label::textColourId, Colours::blue);
	infoLabel.setBounds (10, 4, 400, 25);

	addAndMakeVisible (thumbnailTest = new DemoThumbnailComp());
	thumbnailTest->setBounds(50, 150, 300, 100);
	//addAndMakeVisible (testWaveform = new WaveformView());
	//testWaveform->setBounds(50, 150, 300, 100);

    setSize (500, 300);
   
    startTimer (50);
}

JuceDemoPluginAudioProcessorEditor::~JuceDemoPluginAudioProcessorEditor()
{
}

//==============================================================================
void JuceDemoPluginAudioProcessorEditor::paint (Graphics& g)
{
    //g.setGradientFill (ColourGradient (Colours::white, 0, 0, Colours::red, 0, (float) getHeight(), false));
	g.fillAll(Colours::white);
    //g.fillAll();

	//g.setColour (Colours::mediumblue);

 //   if (thumbnail.getTotalLength() > 0)
 //   {
 //       thumbnail.drawChannels (g, getLocalBounds().reduced (2, 2),
	//		0, thumbnail.getTotalLength(), 1.0f);
 //   }
 //   else
 //   {
 //       g.setFont (14.0f);
 //       g.drawFittedText ("(No audio file selected)", 0, 0, getWidth(), getHeight(),
 //                           Justification::centred, 2);
 //   }
}


//==============================================================================
// This timer periodically checks whether any of the filter's parameters have changed...
void JuceDemoPluginAudioProcessorEditor::timerCallback()
{
    JuceDemoPluginAudioProcessor* ourProcessor = getProcessor();

    AudioPlayHead::CurrentPositionInfo newPos (ourProcessor->lastPosInfo);

    if (lastDisplayedPosition != newPos)
        displayPositionInfo (newPos);

    delaySlider.setValue (ourProcessor->delay, false);
}

// This is our Slider::Listener callback, when the user drags a slider.
void JuceDemoPluginAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    if (slider == &delaySlider)
    {
        getProcessor()->setParameterNotifyingHost (JuceDemoPluginAudioProcessor::delayParam,
                                                   (float) delaySlider.getValue());
    }
}

void JuceDemoPluginAudioProcessorEditor::buttonClicked(Button* btn)
{
	if(btn == &loadButton){

		FileChooser myChooser ("Please choose a file:", File::getSpecialLocation(File::userDesktopDirectory), "*.wav");
		if(myChooser.browseForFileToOpen())
		{			
			testFile = myChooser.getResult();
			//loadFileIntoTransport(testFile);
			//testWaveform->setFile(testFile);
			thumbnailTest->setFile(testFile);
			//String str = String(thumbnail.getTotalLength());
			//helloLabel.setText(str, false);
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
void JuceDemoPluginAudioProcessorEditor::displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos)
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
