Code Overview for Developers
============================


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
This contains the properties of a SampleStripControl, things like the current sample, how much of it is selected etc. Any changes to a SampleStrip control should update the corresponding SampleStrip.



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