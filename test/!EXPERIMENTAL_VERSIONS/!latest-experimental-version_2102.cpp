#include <stdint.h>
#include <Arduino.h>
#include <MIDI.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <TCA9548.h>
#include "notes.h"
#include <IntervalTimer.h>

#define NUM_VOICES 8
#define MIDI_CHANNEL 1
#define DETUNE 0
#define PITCH_BEND_RANGE 2
//#define LFO_PIN 40

float pitchBenderValue = 8192;
float prevPitchBenderValue = 8192;
float bendFactor = 0;
bool susOn = false;
uint8_t midiNote = 0;
uint8_t velocity = 0;
uint8_t aftertouch = 0;
uint8_t modulationWheel = 0;
uint8_t ccNumber = 0;
uint8_t ccValue = 0;
uint8_t sustainPedal = 0;
uint8_t knobNumber = 0;
uint8_t knobValue = 0;
uint8_t knob[17];
int midiNoteVoltage = 0;
int portaSpeed = 0;

// ----------------------------- DCO vars
const int FSYNC_PINS[8] = {2, 3, 4, 5, 6, 7, 8, 9};
#define SPI_CLOCK_SPEED 7500000                     // 7.5 MHz SPI clock - this works ALMOST without clock ticks
unsigned long MCLK = 25000000;      

// ----------------------------- Voice struct
  struct Voice {
    unsigned long noteAge;
    uint8_t midiNote;
    bool noteOn;
    bool sustained;
    bool keyDown;
    uint8_t velocity;
    uint16_t noteVolts;
    float noteFreq;
    float prevNoteFreq;
    float prevNoteDiff;
    unsigned long prevNoteAge;
    bool portaOn;
    float portaStepSize;
    unsigned long lastPortaStep;
  };

Voice voices[NUM_VOICES];

void initializeVoices() {
  for (int i = 0; i < NUM_VOICES; i++) {
    voices[i].noteAge = 0;
    voices[i].midiNote = 0;
    voices[i].noteOn = false;
    voices[i].sustained = false;
    voices[i].keyDown = false;
    voices[i].velocity = 0;
    voices[i].noteVolts = 0;
    voices[i].noteFreq = 0;
    voices[i].prevNoteFreq = 0;
    voices[i].prevNoteDiff = 0;
    voices[i].prevNoteAge = 0;
    voices[i].portaOn = false;
    voices[i].portaStepSize = 0;
    voices[i].lastPortaStep = 0;
    }
}

// ------------------------ Debug Print
void debugPrint(int voice) {
  Serial.print("Voice #" + String(voice));
  Serial.print("  Key: ");
  Serial.print(voices[voice].midiNote);
  Serial.print("\tFreq: ");
  Serial.print(noteFrequency[voices[voice].midiNote]);
  Serial.print(" -> ");
  Serial.print(voices[voice].noteFreq);
  Serial.print("\tOut: ");
  Serial.print(voices[voice].noteFreq);
  Serial.print("\tkeyDown: ");
  Serial.print(voices[voice].keyDown);
  Serial.print("\tOn: ");
  Serial.print(voices[voice].noteOn);
  Serial.print("\t -> Sustained: ");
  Serial.println(voices[voice].sustained); 
  //Serial.println(bendFactor);
}

// ***********************************************************************
// **************************** DCO routines *****************************
// ***********************************************************************

// ------------------------ Output voice

void AD9833Reset(int AD_board) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  int FSYNC_RESET_PIN = FSYNC_PINS[AD_board];
  digitalWrite(FSYNC_RESET_PIN, LOW);
  SPI.transfer16(0x2100);
  digitalWrite(FSYNC_RESET_PIN, HIGH);
}

// ------------------------ Update voice
void updateVoices() {
  for (int i = 0; i < NUM_VOICES; i++) {
    float ratio = pow(2, 1 / 12.0);
    bendFactor = map(pitchBenderValue, 0, 16383, -PITCH_BEND_RANGE, PITCH_BEND_RANGE);
    if (voices[i].noteOn == true) {
      voices[i].noteFreq = noteFrequency[voices[i].midiNote] * pow(ratio, bendFactor) + voices[i].portaStepSize;
      long FreqReg0 = (voices[i].noteFreq * pow(2, 28)) / MCLK;   // Data sheet Freq Calc formula
      int MSB0 = (int)((FreqReg0 & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
      int LSB0 = (int)(FreqReg0 & 0x3FFF);
      int FSYNC_SET_PIN = FSYNC_PINS[i];
      SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
      digitalWrite(FSYNC_SET_PIN, LOW);                         // set FSYNC low before writing to AD9833 registers
      LSB0 |= 0x4000;                                    // DB 15=0, DB14=1
      MSB0 |= 0x4000;                                    // DB 15=0, DB14=1
      SPI.transfer16(LSB0);                              // write lower 16 bits to AD9833 registers
      SPI.transfer16(MSB0);                              // write upper 16 bits to AD9833 registers
      SPI.transfer16(0xC000);                           // write phase register
      SPI.transfer16(0x2002);                           // take AD9833 out of reset and output triangle wave (DB8=0)
      delayMicroseconds(2);                             // Settle time? Doesn't make much difference …
      digitalWrite(FSYNC_SET_PIN, HIGH);                        // write done, set FSYNC high
      SPI.endTransaction();
    }
  }
}

// ------------------------ Calculate portamento steps

void portaStep() {
  if (portaSpeed > 10) {  // safety margin for crappy knobs & faders
    for (int i = 0; i < NUM_VOICES; i++) {
      if (voices[i].portaOn == true) {
        float deltaTime = voices[i].noteAge - voices[i].prevNoteAge; // time diff between current and previous note
        float portaTimeMs = 5000.0f - (portaSpeed / 127.0f) * 4500.0f;  // time slice depending on porta speed
        float portaFreqDiff = voices[i].prevNoteFreq - voices[i].noteFreq;
        voices[i].portaStepSize = portaFreqDiff * (deltaTime / portaTimeMs);
      }
    }
  }
}

// ------------------------ Voice buffer routines

int findOldestVoice() {
  int oldestVoice = 0;
  unsigned long oldestAge = 0xFFFFFFFF;
  for (int i = 0; i < NUM_VOICES; i++) {
    if (!voices[i].noteOn && voices[i].noteAge < oldestAge) {
      oldestVoice = i;
      oldestAge = voices[i].noteAge;
    }
  }
  return oldestVoice;
}

int findVoice(uint8_t midiNote) {
  int foundVoice = -1;
  for (int i = 0; i < NUM_VOICES; i++) {
    if (voices[i].noteOn && voices[i].midiNote == midiNote) {
      foundVoice = i;
      break;
    }
  }
  return foundVoice;
}

void noteOn(uint8_t midiNote, uint8_t velocity) {
    
  int voice = findVoice(midiNote);
  if (voice == -1) {
    int numPlayingVoices = 0;
    for (int i = 0; i < NUM_VOICES; i++) {
      if (voices[i].noteOn) {
        numPlayingVoices++;
        //AD9833setFrequency(i, noteFrequency[voices[i].midiNote]); // not sure I need this here ...
      }
    }
    if (numPlayingVoices >= NUM_VOICES) {
      unsigned long oldestAge = 0xFFFFFFFF;
      int oldestVoice = -1;
      for (int i = 0; i < NUM_VOICES; i++) {
        if (voices[i].noteAge < oldestAge) {
          oldestAge = voices[i].noteAge;
          oldestVoice = i;
        }
      }
      voice = oldestVoice;
    } else {
      for (int i = 0; i < NUM_VOICES; i++) {
        if (!voices[i].noteOn) {
          voice = i;
          break;
        }
      }
    }
  }
  voices[voice].noteAge = millis();
  voices[voice].midiNote = midiNote;
  voices[voice].noteOn = true;
  voices[voice].keyDown = true;
  voices[voice].velocity = velocity;
  voices[voice].noteFreq = noteFrequency[voices[voice].midiNote];
  voices[voice].prevNoteDiff = voices[voice].noteFreq - voices[voice].prevNoteFreq;
  updateVoices();
}

void noteOff(uint8_t midiNote) {
  int voice = findVoice(midiNote);
  if (voice != -1) {
    voices[voice].keyDown = false;
    if (susOn == false) {
      voices[voice].prevNoteFreq = voices[voice].noteFreq;
      voices[voice].prevNoteAge = voices[voice].noteAge;
      voices[voice].noteOn = false;
      voices[voice].velocity = 0;
      voices[voice].midiNote = 0;
      voices[voice].noteAge = 0;
      voices[voice].noteFreq = 0;
    }
  }
}

void unsustainNotes() {
  for (int i = 0; i < NUM_VOICES; i++) {
    voices[i].sustained = false;
    if (voices[i].keyDown == false) {
      voices[i].noteOn = false;
      voices[i].velocity = 0;
      voices[i].midiNote = 0;
      voices[i].noteAge = 0;
      voices[i].noteFreq = 0;
    }
  }
}

void sustainNotes() {
  for (int i = 0; i < NUM_VOICES; i++) {
    if (voices[i].noteOn == true) {
      voices[i].sustained = true;
    }
  }
}

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1,  MIDI);
IntervalTimer portaTimer;

// ************************************************
// ******************** SETUP *********************
// ************************************************

void setup() {
  portaTimer.begin(&portaStep, 2000);
	Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL);
  SPI.begin();
  for (int i = 0; i < NUM_VOICES; i++) {
    int FSYNC_PIN_INIT = FSYNC_PINS[i];
    pinMode(FSYNC_PIN_INIT, OUTPUT);                           
    digitalWrite(FSYNC_PIN_INIT, HIGH); 
    AD9833Reset(i);
    }                     
}

// ************************************************
// ******************** MAIN **********************
// ************************************************

void loop() {

  if (MIDI.read()) {
  
    // -------------------- Note On
    if (MIDI.getType() == midi::NoteOn && MIDI.getChannel() == MIDI_CHANNEL) {
      midiNote = MIDI.getData1();
      velocity = MIDI.getData2();
      noteOn(midiNote, velocity);
    }
    
    // -------------------- Note Off
    if (MIDI.getType() == midi::NoteOff && MIDI.getChannel() == MIDI_CHANNEL) {
      midiNote = MIDI.getData1();
      noteOff(midiNote);
    }

    // ------------------ Pitchbend 
    if (MIDI.getType() == midi::PitchBend && MIDI.getChannel() == MIDI_CHANNEL) {
      prevPitchBenderValue = pitchBenderValue;
      pitchBenderValue = MIDI.getData2() << 7 | MIDI.getData1(); // already 14 bits = Volts out
      updateVoices();
    }

    // ------------------ Aftertouch 
    if (MIDI.getType() == midi::AfterTouchChannel && MIDI.getChannel() == MIDI_CHANNEL) {
      aftertouch = MIDI.getData1();
    }

    // ------------------ Modwheel 
    if (MIDI.getType() == midi::ControlChange && MIDI.getData1() == 1 && MIDI.getChannel() == MIDI_CHANNEL) {
      modulationWheel = MIDI.getData2();
		}

		// ------------------ Sustain
    if (MIDI.getType() == midi::ControlChange && MIDI.getData1() == 64 && MIDI.getChannel() == MIDI_CHANNEL) {
      sustainPedal = MIDI.getData2();
      if (sustainPedal > 63) {
        susOn = true;
        sustainNotes();
      } 
      if (sustainPedal <= 63) {
        susOn = false;
        unsustainNotes();
      }
    }

    // ------------------ MIDI CC
    if (MIDI.getType() == midi::ControlChange && MIDI.getChannel() == MIDI_CHANNEL) {
      knobNumber = MIDI.getData1();
      knobValue = MIDI.getData2();
      if (knobNumber == 73) {
        portaSpeed = knobValue;
      }
    }
  }

  // ****************************************************************
  // *************************** OUTPUT *****************************
  // ****************************************************************
  
}
