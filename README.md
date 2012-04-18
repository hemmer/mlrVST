mlrVST
======

VST port of the popular monome app using JUCE. At this stage the app is very much in the alpha phase, so for the love of god, please make sure your speaker are on low volume and pop a limiter / compressor on the track with the VST. I have had a couple of nasty clicks with some samples so watch out (you are responsible for your equipment!).

Things that might lead to odd behaviour:

- changing internal tempo (and possibly other global settings) then closing and reopening the GUI

Features / Status
-----------------

- Framework for sample manipulation (several playmodes implemented)
- Can use host BPM or internal tempo (can calculate playback speeds accordingly)
- Handles OSC messages to and from the monome
- Number of playback channels can be set at runtime (has no difficult running 8 simultaneous channels of audio)
- Native Linux build works (kindof)!
- Can load .wav, .flac, .ogg, .aiff, .caf, with arbitrary sample rates
- Quantisation of button presses
- Recording / Resampling (though needs to become more user friendly)

Latest Screenshot
-----------------

![Latest Screenshot](/hemmer/mlrVST/raw/master/Screenshots/Latest.png)

(Possible) Upcoming Features
-------------------------
- MIDI / OSC mapping for all controls
- ADSR envelopes for samples (maybe make advanced options).
- Load MIDI tracks into strips
- Pattern recorder
- Speed independent pitch control of samples
- 64 bit version?
- proper sample management

Downloads
---------

Both Win / Mac versions are available from [here](/hemmer/mlrVST/downloads).

Links of Interest
-----------------

- [monome](http://monome.org)
- [forum thread](http://post.monome.org/comments.php?DiscussionID=12988)
- [mlrV](http://parallelogram.cc/mlrv/)




