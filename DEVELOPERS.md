# Code Overview for Developers #

**mlrVST** is written using the cross-platform JUCE library in C++. As as result the code should work on Win, Mac and Linux without too much trouble, though a couple of minor changes may need to be made.


## Compiling ##

### General ###

First you will need to download both the [latest version of JUCE](https://github.com/julianstorer/JUCE) and the the VST SDK from Steinburg (vstsdk2.4). JUCE uses an internal program called the Introjucer to produce build targets and generally manage the project. The first step should be to build & run this (it can be build using the files in \juce\extras\Introjucer\Builds).

Once this is done, you can open the *mlrVST.jucer* project file using the Introjucer to inspect the project settings etc. New files should be added here so they can be added to all of the build targets. Also at this point you should make sure that the paths to JUCE and the VST SDK are correctly specified.


### Win ###

Just open the .sln file in the *builds/Visual Studio 2010* folder and you should be ready to go. After a successful compilation, there should be a file *mlrVST.dll* in the Debug (or Release) folders - this can be added to your host's VST folder.

### Mac ###

First open the *Builds/MacOSX/mlrVST.xcodeproj* project file to load the project into Xcode. Apple's LLVM compiler finds an error with one of the VST files *audioeffect.cpp* which you can simply autofix. Alternatively, use the GCC compiler. If you want to build AU plugins, you will need to [follow the instructions here](http://www.rawmaterialsoftware.com/viewtopic.php?f=8&t=8682).

### Linux ###

Apparently builds fine. There is a Makefile generated, but I've not checked this thoroughly yet.



## Guide to Key Classes ##

### Audio Processing ###

#### PluginProcessor ####

The guts of the application are in this class. This is what deals with the VST code, handling the overall audio processing. As the PluginEditor is destroyed when you close the GUI, everything that wants to be persistant must be saved here. This is why you may see what looks like redundancy between similar objects (why we need SampleStrip and SampleStripControl for example). The sample pools are also stored in this class, i.e. the objects which store samples in memory.

#### SampleStrip ####

Each row on the monome controls a SampleStrip. This class handles the audio processing for that Strip, and stores the relevent parameters (e.g. playspeed, volume). Any changes to a SampleStripControl (the GUI analog) should update the corresponding SampleStrip parameters using setSampleStripParam. 

### GUI ###

#### PluginEditor ####

This is the main GUI class that handles drawing the components, anything visual really. This should really just forward any commands to the PluginProcessor class and not do very much itself save for handling user input.

#### SampleStripControl ####

These represent the rows of the monome that are used to manipulate samples. This class allows the user to use the GUI to change parameters (such as volume, the selection of the sample etc), and does so by relaying this information to the SampleStrips.



### Other ###

#### OSCHandler ####

Basic class which processes any OSC messages to / from the monome.



## General points ##

Every time the GUI (PluginEditor) is closed in the host, its destructor is called. This means that everything that needs to persist must originate in the PluginProcessor class (hence why AudioSamples and SampleStrips are stored there).

This is still in alpha, I have found in windows that if a Release build hangs in your VST, a restart may be required. If you're running the plugin from Visual Studio (using the PluginHost.exe included in utilities) then you can handle crashes without needing a restart.