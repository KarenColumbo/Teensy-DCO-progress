# Teensy-DCO8: 8 voices polyphonic AD9833 triangle wave DCO for Teensy 4.1

### Features:

- 8 voices polyphonic note buffer with oldest note stealing
- Fine tuning (atm only once in the setup routine), default 440 Hz
- full polyphonic portamento/glide - somewhat chaotic, though
- hold/sustain pedal logic
- pitchbend included
- frequency CVs, velocities, gates, pitchbender output though a couple of MCP4728 via a TCA9548

I'm calling a voice update subroutine in every cycle of the main loop, portamento gets included via timer interrupt. This may not be ideal, and my n00b code reflects it.

There's quite an awful lot of room for improvement for the time being, but I'll get better with time.

To do:

- LFO input OR soft LFO function that additionally gets put out as a CV somewhere.
- Get MIDI note frequencies calculated "on the fly" instead of an array generated at startup - with better coding skills I could get that to work, but I tried, and the pitchbending got sluggish. Gotta rework the complete voice update interrupt.
