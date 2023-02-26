#include <stdint.h>
#include <Arduino.h>
#include <MIDI.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <TCA9548.h>
//#include "notes.h"
#include <IntervalTimer.h>

#define POLYPHONY 8
#define MIDI_CHANNEL 1
#define DETUNE 0
#define PITCH_BEND_RANGE 2

float noteFrequency[96];
float pitchBenderValue = 8192;
float pitchBend = 0;
uint16_t pitchBenderVolt = 0;
float prevPitchBenderValue = 8192;
float bendFactor = 0;
float ratio = 0;
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
uint8_t portaSpeed = 0;
bool trig = false;
int startNote = 12;
int endNote = 108;
double semitone = pow(2, 1 / 12);
const int adcPin = A10; // pin 24
const double lfoFactor = 2.0; // maximum LFO pitch change in semitones
float lfoBend = 0;
int adcValue = 0;
double tuningFrequency = 440.0; // A4 = 440 Hz

// ----------------------------- DCO vars
const int FSYNC_PINS[8] = {2, 3, 4, 5, 6, 7, 8, 9};
#define SPI_CLOCK_SPEED 25000000  // 7.5 MHz SPI clock - this works ALMOST without clock ticks
unsigned long MCLK = 25000000;      

// ----------------------------- DAC vars
#define TCA_ADDR 0x70
#define MCP_CMD 0x40
const int DAC_CHAN[8] = {0, 1, 2, 3, 0, 1, 2, 3};
const int TCA_CHAN[32] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7};
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
  float portaFreq;
  float dcoFreq;
};

Voice voices[POLYPHONY];

void initializeVoices() {
  for (int i = 0; i < POLYPHONY; i++) {
    voices[i].noteAge = 0;
    voices[i].midiNote = 0;
    voices[i].noteOn = false;
    voices[i].sustained = false;
    voices[i].keyDown = false;
    voices[i].velocity = 0;
    voices[i].noteVolts = 0;
    voices[i].noteFreq = 0;
    voices[i].prevNoteFreq = 0;
    voices[i].portaFreq = 0;
    voices[i].dcoFreq = 0;
  }
}

// ------------------------ TCA/MCP subroutine
void writeMCP4728(byte tcaChannel, byte mcpChannel, int data) {
  // Select TCA9548A channel
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << tcaChannel);
  Wire.endTransmission();

  // Write to MCP4728
  Wire.beginTransmission(0x60 | mcpChannel);
  Wire.write(MCP_CMD);
  Wire.write(data >> 8);
  Wire.write(data & 0xFF);
  Wire.endTransmission();
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
  delayMicroseconds(10); // Wait for 10 us after reset
}

void updateDAC() {
  for (int i = 0; i < POLYPHONY; i++) {
    writeMCP4728(TCA_CHAN[i], DAC_CHAN[i], (pow(2, 12) - 1) * (log2(voices[i].dcoFreq/32.7032) / log2(2093.0045/32.7032)));
    writeMCP4728(TCA_CHAN[i + 8], DAC_CHAN[i], voices[i].velocity);
    writeMCP4728(TCA_CHAN[i + 16], DAC_CHAN[i], voices[i].noteOn ? 4095 : 0);
  }
  writeMCP4728(6, 0, aftertouch);
  writeMCP4728(6, 1, modulationWheel);
  writeMCP4728(6, 2, pitchBenderVolt);
}

// ------------------------ Calculate portamento steps

void portaStep() {  
  trig = false;
  if (portaSpeed > 0) {
    float speed = map(portaSpeed, 0, 127, 0, 32);
    for (int i = 0; i < POLYPHONY; i++) {
      float startF = voices[i].prevNoteFreq;
      float portaF = voices[i].portaFreq;
      float endF = voices[i].noteFreq;
      float portaStep = abs(startF - endF) / (portaF * 0.1225) / speed * 20; 
      if (portaF != 0 && voices[i].noteOn == true) {
        if (portaF != endF) {
          trig = true; 
          if (startF > 0) {
            if (startF < endF) {
              portaF += portaStep;
              if (portaF >= endF) {
                portaF = endF;
              } 
            }
            if (startF > endF) {
              portaF -= portaStep;
              if (portaF <= endF) {
                portaF = endF;
              }
            } 
          }
        voices[i].portaFreq = portaF;
        }
      }
      voices[i].dcoFreq = voices[i].portaFreq;
    }
  }
}

// ------------------------ Voice buffer routines

int findOldestVoice() {
  int oldestVoice = 0;
  unsigned long oldestAge = 0xFFFFFFFF;
  for (int i = 0; i < POLYPHONY; i++) {
    if (!voices[i].noteOn && voices[i].noteAge < oldestAge) {
      oldestVoice = i;
      oldestAge = voices[i].noteAge;
    }
  }
  return oldestVoice;
}

int findVoice(uint8_t midiNote) {
  int foundVoice = -1;
  for (int i = 0; i < POLYPHONY; i++) {
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
    for (int i = 0; i < POLYPHONY; i++) {
      if (voices[i].noteOn) {
        numPlayingVoices++;
      }
    }
    if (numPlayingVoices >= POLYPHONY) {
      unsigned long oldestAge = 0xFFFFFFFF;
      int oldestVoice = -1;
      for (int i = 0; i < POLYPHONY; i++) {
        if (voices[i].noteAge < oldestAge) {
          oldestAge = voices[i].noteAge;
          oldestVoice = i;
        }
      }
      voice = oldestVoice;
    } else {
      for (int i = 0; i < POLYPHONY; i++) {
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
  voices[voice].portaFreq = voices[voice].prevNoteFreq;
  voices[voice].dcoFreq = voices[voice].noteFreq;
  trig = true;
}

void noteOff(uint8_t midiNote) {
  int voice = findVoice(midiNote);
  if (voice != -1) {
    voices[voice].keyDown = false;
    if (susOn == false) {
      voices[voice].noteOn = false;
      voices[voice].velocity = 0;
      voices[voice].midiNote = 0;
      voices[voice].noteAge = 0;
      voices[voice].prevNoteFreq = voices[voice].noteFreq;
      voices[voice].noteFreq = 0;
    }
  }
}

void unsustainNotes() {
  for (int i = 0; i < POLYPHONY; i++) {
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
  for (int i = 0; i < POLYPHONY; i++) {
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
  for (int i = startNote; i <= endNote; i++) {
    int noteIndex = i - startNote; // Calculate the index of the note in the array
    noteFrequency[noteIndex] = pow(2.0, (i - 69.0) / 12.0) * tuningFrequency;
  }
  pinMode(adcPin, INPUT);
  Wire.begin();
  Wire.setClock(400000); // Set the I2C clock frequency to 400 kHz
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(0x00); // Turn on internal pullup resistors
  Wire.endTransmission();
  portaTimer.begin(&portaStep, 20000);
	Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL);
  SPI.begin();
  for (int i = 0; i < 8; i++) {
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
      pitchBenderValue = MIDI.getData2() << 7 | MIDI.getData1(); 
      pitchBenderVolt = map(pitchBenderValue, 0, 16383, 0, 4095);
      pitchBend = pow(semitone, map(pitchBenderValue, 0, 16383, -PITCH_BEND_RANGE, PITCH_BEND_RANGE));
      trig = true; // Check timing with portaStep routine!!
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

  adcValue = analogRead(adcPin);
  lfoBend = pow(semitone, map(adcValue, 0, 4095, -lfoFactor, lfoFactor));

  // ****************************************************************
  // *************************** OUTPUT *****************************
  // ****************************************************************

  if (trig == true) {
    for (int i = 0; i < POLYPHONY; i++) {
      if (voices[i].noteOn == true) {
        voices[i].dcoFreq = noteFrequency[voices[i].midiNote] * pitchBend * lfoBend;
        long FreqReg0 = voices[i].dcoFreq * 268435456 / MCLK;   // Data sheet Freq Calc formula
        int MSB0 = (int)((FreqReg0 & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
        int LSB0 = (int)(FreqReg0 & 0x3FFF);
        int FSYNC_SET_PIN = FSYNC_PINS[i];
        SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
        digitalWrite(FSYNC_SET_PIN, LOW);  // set FSYNC low before writing to AD9833 registers
        LSB0 |= 0x4000; // DB 15=0, DB14=1
        MSB0 |= 0x4000; // DB 15=0, DB14=1
        delayMicroseconds(1); // Wait for 1 us after FSYNC falling edge
        SPI.transfer16(LSB0); // write lower 16 bits to AD9833 registers
        delayMicroseconds(50); // Wait for 50 us between writes                             
        SPI.transfer16(MSB0);  // write upper 16 bits to AD9833 registers
        delayMicroseconds(50); // Wait for 50 us between writes                            
        SPI.transfer16(0xC000); // write phase register
        delayMicroseconds(50); // Wait for 50 us between writes                          
        SPI.transfer16(0x2002);  // take AD9833 out of reset and output triangle wave (DB8=0)
        delayMicroseconds(5); // Wait for 50 us between writes         
        digitalWrite(FSYNC_SET_PIN, HIGH);  // write done, set FSYNC high
        SPI.endTransaction();
      }
    }
  }
  trig = false;
}
