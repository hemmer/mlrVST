Code Overview for Developers
============================

First a few general points:

Every time the GUI (PluginEditor) is closed in the host, its destructor is called. This means that everything that needs to persist must originate in the PluginProcessor class (hence why AudioSamples and SampleStrips are stored there).

The OSCpack library is cross platform but as yet I haven't figured out how to include it in a JUCE project in a way that Visual Studio is happy. So for now if you are on Mac, you will need to open the Jucer, remove the oscpack/ip/win32 folder and drag the oscpack/ip/posix folder into the project and Save (this will regenerate the Xcode project files).

This is still in alpha, I have found in windows that if a Release build hangs, a restart is required! If you're running in in Visual Studio (using the PluginHost.exe included in utilities) then you can handle crashes without needing a restart.

Compiling
=========

Win
---

You need to download both JUCE and the vstsdk2.4 (available from Steinberg or elsewhere). Then open the file mlrVST.jucer in the Introjucer (downloads on SourceForge) and edit so that the paths to JUCE and the vstsdk are correct. Save then open in VisualStudio and compile!



Audio Processing
================

PluginProcessor
---------------
The guts of the application are in this class. This is what deals with the VST code, handling the overall audio processing. As the PluginEditor is destroy when you close the GUI, everything that wants to be persistant must be saved here. This is why you may see what looks like redundancy (why we need SampleStrip and SampleStripControl for example). The sample pool is also stored in this class.

ChannelProcessor
----------------
This handles the actual sample manipulation for each channel. If we have four seperate channels of audio, we have four ChannelProcessors objects.

SampleStrip
-----------
This contains the properties of a SampleStripControl, things like the current sample, how much of it is selected etc. Any changes to a SampleStrip control should update the corresponding SampleStrip. SampleStripControl communicates with the sample strip using a Parameter enum through intermediate functions in PluginEditor and PluginProcessor.



GUI
===

PluginEditor 
------------
This is the main GUI class that handles drawing the components, anything visual really. This should just forward any logic to the PluginProcessor class.

SampleStripControl
------------------
These are the waveform strips which map onto each row.



Other
=====

OSCHandler
----------
This processes any OSC messages to / from the monome.