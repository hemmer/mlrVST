mlrVST
======

VST port of the popular monome app using JUCE. At this stage the app is very much in the alpha phase, so for the love of god, please make sure your speaker are on low volume and pop a limiter / compressor on the track with the VST. I have had a couple of nasty clicks with some samples so watch out (you are responsible for your equipment!).

Things that might crash:

- Changing a strip's channel while playing 



Current Status
--------------

UI is starting to take shape (at least in terms of functionality). Variable number of channels can be set at runtime.
Basic sample playback can be done. Incoming OSC messages can trigger samples.


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

[mlrV](http://parallelogram.cc/mlrv/)




