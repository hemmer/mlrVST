mlrVST
======

VST port of the popular monome app using JUCE. At this stage the app is in the beta phase, so please test at your own risk (sure your speaker are on low volume and pop a limiter / compressor on the track with the VST if you're paranoid). I have had a couple of nasty clicks in the past, but nothing recently.

Features / Status
-----------------

- Framework for sample manipulation (several playmodes implemented)
- Can use host BPM or internal tempo (can calculate playback speeds accordingly)
- Handles OSC messages to and from the monome
- Number of playback channels can be set at runtime (has no difficult running 8 simultaneous channels of audio)
- Can load .wav, .flac, .ogg, .aiff, .caf, with arbitrary sample rates
- Varying levels of quantisation of button presses
- Basic mapping system for top row / with modifier buttons
- Recording / Resampling / Pattern Recording(though needs to become more user friendly)
- Win / Mac / Linux versions

Latest Screenshot
-----------------

![Latest Screenshot](https://raw.github.com/hemmer/mlrVST/master/Screenshots/Latest.png)

(Possible) Upcoming Features
-------------------------
- MIDI / OSC mapping for all controls
- ADSR envelopes for samples (maybe make advanced options).
- Load MIDI tracks into strips
- Speed independent pitch control of samples
- 64 bit version?
- proper sample management

Downloads
---------

Both Win / Mac versions are available from [here](/hemmer/mlrVST/tree/master/Releases). Choose _release for (probably) more stable, and _bleeding for the latest build.

Links of Interest
-----------------

- [monome](http://monome.org)
- [forum thread](http://post.monome.org/comments.php?DiscussionID=12988)
- [mlrV](http://parallelogram.cc/mlrv/)
- [personal site](http://ewanhemingway.co.uk)




