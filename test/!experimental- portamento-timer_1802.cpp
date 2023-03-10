// At the moment notes that are off don't get pitchbent because pitchbend in the ADSR release phase is ugly. Maybe put a switch in: "Releasebend on/off"?

#include <stdint.h>
#include <Arduino.h>
#include <MIDI.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <IntervalTimer.h>
#include <algorithm>

#define NUM_VOICES 4
#define MIDI_CHANNEL 1
#define DETUNE 0
#define PITCH_BEND_RANGE 2
#define LFO_PIN 40

float pitchBenderValue = 8192;
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
uint8_t knobNumbers[17];
int midiNoteVoltage = 0;
uint8_t portaSpeed = 0;

// ----------------------------- DCO vars
const int FSYNC_PINS[4] = {6, 7, 8, 9};
#define SPI_CLOCK_SPEED 7500000                     // 7.5 MHz SPI clock - this works ALMOST without clock ticks
unsigned long MCLK = 25000000;      

// ----------------------------- MIDI note frequencies C1-C7
float noteFrequency[73] = {
  32.7032, 34.6478, 36.7081, 38.8909, 41.2034, 43.6535, 46.2493, 48.9994, 51.9131, 55.0000, 58.2705, 61.7354, 
  65.4064, 69.2957, 73.4162, 77.7817, 82.4069, 87.3071, 92.4986, 97.9989, 103.8262, 110.0000, 116.5409, 123.4708, 
  130.8128, 138.5913, 146.8324, 155.5635, 164.8138, 174.6141, 184.9972, 195.9977, 207.6523, 220.0000, 233.0819, 246.9417, 
  261.6256, 277.1826, 293.6648, 311.1269, 329.6276, 349.2282, 369.9944, 391.9954, 415.3047, 440.0000, 466.1638, 493.8833, 
  523.2511, 554.3653, 587.3295, 622.2539, 659.2551, 698.4565, 739.9888, 783.9909, 830.6094, 880.0000, 932.3275, 987.7666, 
  1046.5023, 1108.7305, 1174.6591, 1244.5085, 1318.5102, 1396.9129, 1479.9777, 1567.9817, 1661.2188, 1760.0000, 1864.6550, 1975.5332, 
  2093.0045
};

// ----------------------------- 14 bit note frequency voltages C1-C7
const unsigned int noteVolt[73] = {
  0, 15, 32, 49, 68, 87, 108, 130, 153, 177, 203, 231, 
  260, 291, 324, 358, 395, 434, 476, 519, 566, 615, 667, 722, 
  780, 842, 908, 977, 1051, 1129, 1211, 1299, 1391, 1489, 1593, 1704, 
  1820, 1944, 2075, 2214, 2361, 2517, 2682, 2857, 3043, 3239, 3447, 3667, 
  3901, 4148, 4411, 4688, 4982, 5294, 5625, 5974, 6345, 6738, 7154, 7595, 
  8062, 8557, 9081, 9637, 10225, 10849, 11509, 12209, 12950, 13736, 14568, 15450, 
  16383
  };

  struct Voice {
    unsigned long noteAge;
    uint8_t midiNote;
    bool noteOn;
    bool sustained;
    bool keyDown;
    uint8_t velocity;
    float prevNoteFreq;
    float prevNoteAge;
    uint16_t noteVolts;
    float noteFreq;
    float portaDiff;
    float portaStep;
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
    voices[i].prevNoteFreq = 0;
    voices[i].prevNoteAge = 0;
    voices[i].noteVolts = 0;
    voices[i].noteFreq = 0;
    voices[i].portaDiff = 0;
    voices[i].portaStep = 0;
    }
}

// ***********************************************************************
// **************************** DCO routines *****************************
// ***********************************************************************

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

/*void AD9833setFrequency(int board, long frequency0, long frequency1) {
  long FreqReg0 = (frequency0 * pow(2, 28)) / MCLK;   // Data sheet Freq Calc formula
  int MSB0 = (int)((FreqReg0 & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
  int LSB0 = (int)(FreqReg0 & 0x3FFF);
  long FreqReg1 = (frequency1 * pow(2, 28)) / MCLK;
  int MSB1 = (int)((FreqReg1 & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
  int LSB1 = (int)(FreqReg1 & 0x3FFF);
  
  int FSYNC_SET_PIN = FSYNC_PINS[board];
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  digitalWrite(FSYNC_SET_PIN, LOW);                         // set FSYNC low before writing to AD9833 registers

  LSB0 |= 0x4000;                                    // DB 15=0, DB14=1
  MSB0 |= 0x4000;                                    // DB 15=0, DB14=1
  SPI.transfer16(LSB0);                              // write lower 16 bits to AD9833 registers
  SPI.transfer16(MSB0);                              // write upper 16 bits to AD9833 registers

  if (frequency1 > 0) {
    LSB1 |= 0x4000;                                    // DB 15=0, DB14=1
    MSB1 |= 0x4000;                                    // DB 15=0, DB14=1
    SPI.transfer16(LSB1);                              // write lower 16 bits to AD9833 registers
    SPI.transfer16(MSB1);                              // write upper 16 bits to AD9833 registers
  }
  
  SPI.transfer16(0xC000);                           // write phase register
  SPI.transfer16(0x2002);                           // take AD9833 out of reset and output triangle wave (DB8=0)
  delayMicroseconds(2);                             // Settle time? Doesn't make much difference ???

  digitalWrite(FSYNC_SET_PIN, HIGH);                        // write done, set FSYNC high
  SPI.endTransaction();
}*/
void AD9833setFrequency(int board, long frequency0, long frequency1, int waveform1) {
  long FreqReg0 = (frequency0 * pow(2, 28)) / MCLK;   // Data sheet Freq Calc formula
  int MSB0 = (int)((FreqReg0 & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
  int LSB0 = (int)(FreqReg0 & 0x3FFF);
  long FreqReg1 = (frequency1 * pow(2, 28)) / MCLK;
  int MSB1 = 0;
  int LSB1 = 0;
  if (frequency1 > 0) {
    MSB1 = (int)((FreqReg1 & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
    LSB1 = (int)(FreqReg1 & 0x3FFF);
  }
  int FSYNC_SET_PIN = FSYNC_PINS[board];
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  digitalWrite(FSYNC_SET_PIN, LOW);                         // set FSYNC low before writing to AD9833 registers

  LSB0 |= 0x4000;                                    // DB 15=0, DB14=1
  MSB0 |= 0x4000;                                    // DB 15=0, DB14=1
  SPI.transfer16(LSB0);                              // write lower 16 bits to AD9833 registers
  SPI.transfer16(MSB0);                              // write upper 16 bits to AD9833 registers

  if (frequency1 > 0) {
    LSB1 |= 0x4000;                                    // DB 15=0, DB14=1
    MSB1 |= 0x4000;                                    // DB 15=0, DB14=1
    switch (waveform1) {
      case 1:
        MSB1 |= 0x2000;  // set DB13=1 for square wave
        break;
      case 2:
        // set waveform 2 (sine wave), do nothing here as it is the default waveform
        break;
      case 3:
        MSB1 |= 0x2020;  // set DB13=1 and DB5=1 for triangle wave
        break;
      case 4:
        MSB1 |= 0x800;   // set DB11=1 for half-square wave
        break;
      default:
        // invalid waveform type, do nothing here as it is the default waveform
        break;
    }
    SPI.transfer16(LSB1);                              // write lower 16 bits to AD9833 registers
    SPI.transfer16(MSB1);                              // write upper 16 bits to AD9833 registers
  }
  
  SPI.transfer16(0xC000);                           // write phase register
  SPI.transfer16(0x2002);                           // take AD9833 out of reset and output triangle wave (DB8=0)
  delayMicroseconds(2);                             // Settle time? Doesn't make much difference ???

  digitalWrite(FSYNC_SET_PIN, HIGH);                        // write done, set FSYNC high
  SPI.endTransaction();
}

void AD9833Reset(int AD_board) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  int FSYNC_RESET_PIN = FSYNC_PINS[AD_board];
  digitalWrite(FSYNC_RESET_PIN, LOW);
  SPI.transfer16(0x2100);
  digitalWrite(FSYNC_RESET_PIN, HIGH);
}

void bendNotes() {
  for (int i = 0; i < NUM_VOICES; i++) {
    float ratio = pow(2, 1 / 12.0);
    bendFactor = map(pitchBenderValue, 0, 16383, -PITCH_BEND_RANGE, PITCH_BEND_RANGE);
    if (voices[i].noteOn == true) {
      voices[i].noteFreq = noteFrequency[voices[i].midiNote] * pow(ratio, bendFactor);
      AD9833setFrequency(i, voices[i].noteFreq, -1);
    }
    debugPrint(i);
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++ PORTAMENTO & VOICE UPDATE ++++++++++++
//*****************************************************************
float calculatePortaShift() {
  for (int i = 0; i < NUM_VOICES; i++) {
    float deltaTime = (voices[i].noteAge - voices[i].prevNoteAge) / 1000.0f; // Calculate the elapsed time since the previous note event
    // Check if portamento is enabled and calculate the step size accordingly
    if (portaSpeed > 0) {
    float portaSpeedMs = 500.0f + (portaSpeed / 127.0f) * 4500.0f; // Calculate the portamento time in milliseconds
    float portaStep = voices[i].portaDiff * (portaSpeed * deltaTime / portaSpeedMs); // Calculate the step size for this frame
    return voices[i].noteFreq + portaStep; // Calculate the current frequency based on the previous frequency and the portamento step
  } else {
    return voices[i].noteFreq; // Return the current frequency without portamento
  }
}

void updateVoices() {
  /*float deltaTime = 0.001f; // Fixed time interval of 1 millisecond
  
  for (int i = 0; i < NUM_VOICES; i++) {
    if (voices[i].noteOn) {
      float currentFreq = voices[i].noteFreq;
      // ------------------------ Read LFO pin and change note pitch accordingly
      void applyLFO(float& freq) {
      float lfoDepth = (analogRead(LFO_PIN) / 4095.0 * 3.0) / 12.0;
      freq *= pow(2.0, lfoDepth / 12.0); 
      }
      
      // If portamento is enabled, calculate the portamento shift
      if (portaEnabled && voices[i].prevNoteFreq != currentFreq) {
        currentFreq = calculatePortaShift(i);
      }
      
      // Output the frequency for the voice to the appropriate AD9833
      AD9833setFrequency(i, currentFreq, -1);
      
      // Update the fields for the voice
      voices[i].prevNoteFreq = currentFreq;
      voices[i].prevNoteAge = voices[i].noteAge;
    }
  }
}

void timerCallback() {
  updateVoices();*/
}

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
        AD9833setFrequency(i, noteFrequency[voices[i].midiNote], -1);
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
    voices[voice].prevNoteFreq = voices[voice].noteFreq;
  }
  voices[voice].noteAge = millis();
  voices[voice].midiNote = midiNote;
  voices[voice].noteOn = true;
  voices[voice].keyDown = true;
  voices[voice].velocity = velocity;
  voices[voice].noteFreq = noteFrequency[voices[voice].midiNote];
  voices[voice].portaDiff = voices[voice].noteFreq - voices[voice].prevNoteFreq;
  bendNotes();

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

// Sustain management
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

// ************************************************
// ******************** SETUP *********************
// ************************************************

void setup() {
  /*// Initialize the IntervalTimer to trigger the timerCallback() function every 1 millisecond
  IntervalTimer timer;
  timer.begin(timerCallback, 1000); // Trigger every 1 millisecond (1000 microseconds)*/

	Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL);
  SPI.begin();
  for (int i = 0; i < NUM_VOICES; i++) {
    int FSYNC_PIN_INIT = FSYNC_PINS[i];
    pinMode(FSYNC_PIN_INIT, OUTPUT);                           // Prepare FSYNC pin for output
    digitalWrite(FSYNC_PIN_INIT, HIGH); 
    AD9833Reset(i);
    }                     // Set it high for good measure
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
       for (int i = 0; i < NUM_VOICES; i++) {
            debugPrint(i);
        }
    }
    
    // -------------------- Note Off
    if (MIDI.getType() == midi::NoteOff && MIDI.getChannel() == MIDI_CHANNEL) {
      midiNote = MIDI.getData1();
        noteOff(midiNote);
         for (int i = 0; i < NUM_VOICES; i++) {
            debugPrint(i);
        }
    }

    // ------------------ Pitchbend 
    if (MIDI.getType() == midi::PitchBend && MIDI.getChannel() == MIDI_CHANNEL) {
      pitchBenderValue = MIDI.getData2() << 7 | MIDI.getData1(); // already 14 bits = Volts out
      bendNotes();
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
        for (int i = 0; i < NUM_VOICES; i++) {
            debugPrint(i);
        }
      } 
      if (sustainPedal <= 63) {
        susOn = false;
        unsustainNotes();
        Serial.print("SusOff");
        for (int i = 0; i < NUM_VOICES; i++) {
            debugPrint(i);
        }
      }
    }

    // ------------------ MIDI CC
    if (MIDI.getType() == midi::ControlChange && MIDI.getChannel() == MIDI_CHANNEL) {
      int knobNumber = MIDI.getData1();
      int knobValue = MIDI.getData2();
      /*// Check if knobNumber is part of the knobNumbers array
      if (std::find(std::begin(knobNumbers), std::end(knobNumbers), knobNumber) != std::end(knobNumbers)) {
        knob[knobNumber] = knobValue;
      }*/
    }
  }

  // ****************************************************************
  // *************************** OUTPUT *****************************
  // ****************************************************************

}