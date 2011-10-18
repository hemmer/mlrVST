mlrVST
======

VST port of the popular monome app using JUCE. At this stage the app is very much in the alpha phase, so for the love of god, please make sure your speaker are on low volume and pop a limiter / compressor on the track with the VST. I have had a couple of nasty clicks with some samples so watch out (you are responsible for your equipment!).

Things that might lead to odd behaviour:

	- changing internal tempo (and possibly other global settings) then closing and reopening the GUI



Current Status
--------------

- Framework for sample manipulation (several playmodes implemented)
- Follows host BPM and calculates speeds accordingly
- GUI starting to take shape nicely, volume, speed, playmode controls
- Handles OSC messages from the monome (only tested on Windows so far)
- Number of playback channels can be set at runtime (has no difficult running 8 channels of audio)
- Native linux build works (kindof)!
- can load .wavs with arbitrary sample rates (48Hz users rejoice).

Latest Screenshot
-----------------

![Latest Screenshot](/hemmer/mlrVST/raw/master/Screenshots/Latest.png)

Possible Upcoming Features
-------------------------
- ADSR envelopes for samples (maybe make advanced options).
- configurable number of channels
- Also load MIDI tracks into strips
- Speed independent pitch control of samples
- 64 bit version?
- proper sample management

Links of Interest
-----------------

[monome](http://monome.org)
[forum thread](http://post.monome.org/comments.php?DiscussionID=12988)
[mlrV](http://parallelogram.cc/mlrv/)




