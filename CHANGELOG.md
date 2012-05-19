CHANGELOG
=========

beta 0.1.7 (19/05/2012)
-----------
NEW FEATURES

- added envelope option (of variable length) so don't get clicking sound when looping
- added mapping setup for top row
- added multiple modifier buttons that when held turn the rows into control strips
- added TapeStop (tm) mode for stopping samples with a tape effect
- added mutes (click channel number
- updated to use latest version of JUCE library

BUGFIXES

- fixed bug where program would incorrectly think keys were being held
- improved MIDI timing and accuracy (now only limited by system latency)


alpha 0.1.6 (15/04/2012)
-----------
NEW FEATURES

- buttons with visual feedback for easier use of recording / resampling
- added quantisation (with working inner looping and stopping combos)
- added last button looping
- neatened up UI a little bit

BUGFIXES

- visual selection now correctly snaps to 16ths if CTRL-SHIFT is held
- fixed rare memory leak

alpha 0.1.5 (18/12/2011)
-----------

NEW FEATURES

- each strip now acts a control strip when a modifier key on the top row is pressed. basically this allows things like speed and volume to be controlled for each strip without leaving your monome!
- resampling / recording
- better code for drawing waveforms
- added Mac build!

alpha 0.1.4 (18/11/2011)
-----------
- major internal rewrite for more efficient audio processing
- smoother GUI updates


alpha 0.1.3
-----------

- internal/external tempo
- preset working internally (not much use yet though!)
- fixed a couple of playback bugs
- top row now act as stop buttons


