# Teensy-DCO8

## 8 voices polyphonic AD9833 triangle wave DCO for Teensy 4.1

### Progress

- Got a gig in March, so there's no time to get this forward. Will continue in April.

### Features:

- 8 voices polyphonic note buffer with oldest note stealing
- Master tune (atm only once in the setup routine), default 440 Hz
- Fully polyphonic portamento/glide - somewhat chaotic result, though. Crazy stuff
- Hold/sustain pedal logic
- MIDI pitchbend included
- ADC input with scaling for LFO/FM pitch magic
- CVs for note frequencies, velocities, gates, pitchbender, aftertouch, modwheel output though a couple of MCP4728 boards via TCA9548
- Streamlined AD9833 addressing via SPI (no library needed)

I'm calling a voice update subroutine in every cycle of the main loop, portamento gets included via timer interrupt. This may not be ideal, and my n00b code reflects it.

There's quite a bit of room for improvement for the time being, but I'll take my sweet time.

### To do:

- portamento options: farthest, nearest notes
- Note stealing options - highest, lowest note, round robin?
- Glissando
- Couple of MIDI CC knobs and faders written to DACs for analog stuff like VCO/VCF/VCA control
- Get MIDI note frequencies calculated "on the fly" instead of an array generated at startup - with better coding skills I could get that to work, but I tried, and the pitchbending got sluggish. Gotta seriously rework the voice update interrupt.
