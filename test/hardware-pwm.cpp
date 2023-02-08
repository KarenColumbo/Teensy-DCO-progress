//*********************** https://www.pjrc.com/teensy/td_pulse.html ********************************
// Language C/C++
// Board Teensy 4.1
#include <stdint.h>
#include <Arduino.h>
#include <MIDI.h>
#include <Adafruit_MCP4728.h>
#include <SoftwareSerial.h>

#define NUM_VOICES 8
#define MIDI_CHANNEL 1
#define PITCH_POS 2 // Pitchbend range in +/- benderValue
#define PITCH_NEG -2
#define CC_TEMPO 5
#define A4 440

uint8_t midiTempo;
uint8_t midiController[10];
uint16_t benderValue = 0;
bool susOn = false;
int arpIndex = 0;
int numArpNotes = 0;
int arpNotes[NUM_VOICES];
uint16_t eighthNoteDuration = 0;
uint16_t sixteenthNoteDuration = 0; 

// --------------------------------- 12 bit Velocity Voltages - linear distribution
const float veloVoltLin[128]={
  0, 32, 64, 96, 128, 160, 192, 224, 
  256, 288, 320, 352, 384, 416, 448, 480, 
  512, 544, 576, 608, 640, 672, 704, 736, 
  768, 800, 832, 864, 896, 928, 960, 992, 
  1024, 1056, 1088, 1120, 1152, 1184, 1216, 1248, 
  1280, 1312, 1344, 1376, 1408, 1440, 1472, 1504, 
  1536, 1568, 1600, 1632, 1664, 1696, 1728, 1760, 
  1792, 1824, 1856, 1888, 1920, 1952, 1984, 2016, 
  2048, 2080, 2112, 2144, 2176, 2208, 2240, 2272, 
  2304, 2336, 2368, 2400, 2432, 2464, 2496, 2528, 
  2560, 2592, 2624, 2656, 2688, 2720, 2752, 2784, 
  2816, 2848, 2880, 2912, 2944, 2976, 3008, 3040, 
  3072, 3104, 3136, 3168, 3200, 3232, 3264, 3296, 
  3328, 3360, 3392, 3424, 3456, 3488, 3520, 3552, 
  3584, 3616, 3648, 3680, 3712, 3744, 3776, 3808, 
  3840, 3872, 3904, 3936, 3968, 4000, 4032, 4064
};

// --------------------------------- 12 bit Velocity Voltages - log distribution
const float veloVoltLog[128]={
1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 
5, 5, 11, 11, 15, 16, 16, 17, 21, 22, 23, 39, 41, 42, 
48, 49, 55, 57, 59, 66, 93, 96, 104, 106, 115, 118, 127, 130, 
139, 181, 185, 195, 206, 211, 222, 226, 238, 250, 309, 323, 337, 343, 
357, 371, 386, 393, 408, 496, 513, 521, 538, 556, 574, 592, 611, 629, 
741, 762, 783, 804, 826, 848, 870, 892, 915, 1054, 1078, 1103, 1129, 1154, 
1180, 1206, 1245, 1272, 1441, 1470, 1512, 1542, 1572, 1602, 1646, 1677, 1709, 1924, 
1958, 1992, 2041, 2075, 2125, 2161, 2196, 2248, 2500, 2539, 2593, 2633, 2689, 2729, 
2785, 2826, 2884, 3176, 3220, 3282, 3343, 3389, 3451, 3497, 3561, 3626, 3961, 4028
};

// ----------------------------- MIDI note frequencies C1-C7
float midiNoteFrequency [73] = {
  32.7032, 34.6478, 36.7081, 38.8909, 41.2034, 43.6535, 46.2493, 48.9994, 51.9131, 55, 58.2705, 61.7354, 
  65.4064, 69.2957, 73.4162, 77.7817, 82.4069, 87.3071, 92.4986, 97.9989, 103.826, 110, 116.541, 123.471, 
  130.813, 138.591, 146.832, 155.563, 164.814, 174.614, 184.997, 195.998, 207.652, 220, 233.082, 246.942, 
  261.626, 277.183, 293.665, 311.127, 329.628, 349.228, 369.994, 391.995, 415.305, 440, 466.164, 493.883, 
  523.251, 554.365, 587.33, 622.254, 659.255, 698.456, 739.989, 783.991, 830.609, 880, 932.328, 987.767, 
  1046.5, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 1760, 1864.66, 1975.53, 2093
};

// ----------------------------- 14 bit note frequency voltages C1-C7
const unsigned int noteVolt[73] = {
  0, 15, 32, 49, 68, 87, 108, 130, 153, 177, 203, 231, 
  260, 291, 324, 358, 395, 434, 476, 519, 566, 615, 667, 722, 
  780, 842, 908, 977, 1051, 1129, 1211, 1299, 1391, 1489, 1593, 1704, 
  1820, 1944, 2075, 2214, 2361, 2517, 2682, 2857, 3043, 3239, 3447, 3667, 
  3901, 4148, 4411, 4688, 4982, 5294, 5625, 5974, 6345, 6738, 7154, 7595, 
  8062, 8557, 9081, 9637, 10225, 10849, 11509, 12209, 12950, 13736, 14568, 15450, 16383
  };

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// ------------------------------------- Voice buffer init 
struct Voice {
  unsigned long noteAge;
  uint8_t midiNote;
  bool noteOn;
  uint8_t velocity;
  uint16_t pitchBend;
  uint8_t channelPressure;
  uint8_t modulationWheel;
  uint8_t prevNote;
  uint16_t bendedNote;
};

Voice voices[NUM_VOICES];

void initializeVoices() {
  for (int i = 0; i < NUM_VOICES; i++) {
    voices[i].noteAge = 0;
    voices[i].midiNote = 0;
    voices[i].noteOn = false;
    voices[i].velocity = 0;
    voices[i].pitchBend = 0x2000;
    voices[i].channelPressure = 0;
    voices[i].modulationWheel = 0;
    voices[i].prevNote = 0;
    voices[i].bendedNote = 0x2000;
  }
}

// ------------------------------------------ Voice buffer subroutines 
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
    voice = findOldestVoice();
    voices[voice].prevNote = voices[voice].midiNote;
  }
  voices[voice].noteAge = millis();
  voices[voice].midiNote = midiNote;
  voices[voice].noteOn = true;
  voices[voice].velocity = velocity;
}

void noteOff(uint8_t midiNote) {
  int voice = findVoice(midiNote);
  if (voice != -1) {
    voices[voice].noteOn = false;
    voices[voice].velocity = 0;
  }
}

//-------------------------------- Fill Arpeggio buffer from oldest to youngest note
void fillArpNotes() {
  arpIndex = 0;
  for (int i = 0; i < NUM_VOICES; i++) {
    if (voices[i].noteOn) {
      arpNotes[arpIndex++] = voices[i].midiNote;
    }
  }
  numArpNotes = arpIndex;
  for (int i = 0; i < NUM_VOICES; i++) {
    for (int j = 0; j < arpIndex - 1; j++) {
      if (voices[i].noteAge < voices[j].noteAge) {
        int temp = arpNotes[j];
        arpNotes[j] = arpNotes[j + 1];
        arpNotes[j + 1] = temp;
      }
    }
  }
}

// ------------------------------------ Initialize DACs
Adafruit_MCP4728 dac1;
Adafruit_MCP4728 dac2;
Adafruit_MCP4728 dac3;
Adafruit_MCP4728 dac4;

// ******************************************************************************************************
// ************************************************************************ MAIN SETUP 
void setup() {
  for (int i = 0; i < NUM_VOICES; i++) {
    arpNotes[i] = -1;
  }

  // ****************** WARNING: Connect VDD to 5 volts!!! 
  // ****************** DAC Wiring:
  // Teensy 4.1 --> DAC1, DAC2
  // Pin 16 (SCL) --> SCL
  // Pin 17 (SDA) --> SDA
  // Teensy 4.1 --> DAC3, DAC4
  // Pin 20 (SCL1) --> SCL
  // Pin 21 (SDA1) --> SDA
  // Initialize I2C communication
  dac1.begin(0x60);
  dac2.begin(0x61);
  dac1.begin(0x62);
  dac2.begin(0x63);
  Wire.begin(400000);
  
  // Set 14 bits Hardware PWM for pitchbender and 8 note voltage outputs
  analogWriteResolution(14);
  pinMode(2, OUTPUT); // Note 01
  analogWriteFrequency(2, 9155.27);
  pinMode(3, OUTPUT); // Note 02
  analogWriteFrequency(3, 9155.27);
  pinMode(4, OUTPUT); // Note 03
  analogWriteFrequency(4, 9155.27);
  pinMode(5, OUTPUT); // Note 04
  analogWriteFrequency(5, 9155.27);
  pinMode(6, OUTPUT); // Note 05
  analogWriteFrequency(6, 9155.27);
  pinMode(9, OUTPUT); // Note 06
  analogWriteFrequency(7, 9155.27);
  pinMode(22, OUTPUT); // Note 07
  analogWriteFrequency(22, 9155.27);
  pinMode(23, OUTPUT); // Note 08
  analogWriteFrequency(23, 9155.27);
  
  pinMode(10, OUTPUT); // Velocity 01
  pinMode(11, OUTPUT); // Velocity 02
  pinMode(12, OUTPUT); // Velocity 03
  pinMode(13, OUTPUT); // Velocity 04
  pinMode(14, OUTPUT); // Velocity 05
  pinMode(15, OUTPUT); // Velocity 06
  pinMode(18, OUTPUT); // Velocity 07
  pinMode(19, OUTPUT); // Velocity 08

  pinMode(33, OUTPUT); // Pitchbender
  analogWriteFrequency(33, 9155.27);
}

// *****************************************************************************************************
//******************************************** MAIN LOOP *********************************************** 
void loop() {
  
  // ---------------------- Serial MIDI stuff 
  if (MIDI.read()) {

    // ------------------------Check for and buffer incoming Note On message
    if (MIDI.getType() == midi::NoteOn && MIDI.getChannel() == MIDI_CHANNEL) {
      uint8_t midiNote = MIDI.getData1();
      uint8_t velocity = MIDI.getData2();
      if (velocity > 0) {
        noteOn(midiNote, velocity);
      } 
      if (velocity == 0 && susOn == false) {
        noteOff(midiNote);
      }
    }
    
    // ------------------ Check for and write incoming Pitch Bend, map bend factor 
  if (MIDI.getType() == midi::PitchBend && MIDI.getChannel() == MIDI_CHANNEL) {
    uint16_t pitchBend = MIDI.getData1() | (MIDI.getData2() << 7);
    benderValue = map(pitchBend, 0, 16383, PITCH_NEG, PITCH_POS);
    analogWrite(33, benderValue);
  }

  // ----------------------- Check for and write incoming Aftertouch 
  if (MIDI.getType() == midi::AfterTouchChannel && MIDI.getChannel() == MIDI_CHANNEL) {
    uint8_t aftertouch = MIDI.getData1();
    int channelPressurePWM = map(aftertouch, 0, 127, 0, 8191 << 2);
    analogWrite(5, channelPressurePWM);
  }

  // ------------------------- Check for and write incoming Modulation Wheel 
  if (MIDI.getType() == midi::ControlChange && MIDI.getData1() == 1 && MIDI.getChannel() == MIDI_CHANNEL) {
    uint8_t modulationWheel = MIDI.getData2();
    int modulationWheelPWM = map(modulationWheel, 0, 127, 0, 8191 << 2);
    analogWrite(6, modulationWheelPWM);
  }

  // ------------------------- Check for and write incoming MIDI tempo 
  if (MIDI.getType() == midi::ControlChange && MIDI.getChannel() == MIDI_CHANNEL) {
    uint8_t ccNumber = MIDI.getData1();
    uint8_t ccValue = MIDI.getData2();
    if (ccNumber == CC_TEMPO) {
      midiTempo = ccValue;
    }
    eighthNoteDuration = (60 / midiTempo) * 1000 / 2;
    sixteenthNoteDuration = (60 / midiTempo) * 1000 / 4;
  }
    
  // Check for incoming MIDI messages
  if (MIDI.read()) {
    if (MIDI.getType() == midi::ControlChange && MIDI.getChannel() == MIDI_CHANNEL) {
      uint8_t ccNumber = MIDI.getData1();
      uint8_t ccValue = MIDI.getData2();
      if (ccNumber >= 70 && ccNumber <= 79) {
        
      }
    }
  }

  // ---------------------------- Read and store sustain pedal status
  if (MIDI.getType() == midi::ControlChange && MIDI.getData1() == 64 && MIDI.getChannel() == MIDI_CHANNEL) {
    uint8_t sustainPedal = MIDI.getData2();
    if (sustainPedal > 63) {
       susOn = true;
    } else {
       susOn = false;
    }
  }

  // ----------------------- Write gates and velocity outputs, bend notes 
  for (int i = 0; i < NUM_VOICES; i++) {
    digitalWrite(19 - i, voices[i].noteOn ? HIGH : LOW);
    
    // Calculate pitchbender factor
    int midiNoteVoltage = noteVolt[voices[i].midiNote];
    double semitones = (double)benderValue / (double)16383 * 2.0;
    double factor = pow(2.0, semitones / 12.0);
    voices[i].bendedNote = midiNoteVoltage * factor;
    if (voices[i].bendedNote < 0) {
      voices[i].bendedNote = 0;
    }
    if (voices[i].bendedNote > 16383) {
      voices[i].bendedNote = 16383;
    }
  }
  
  // -------------------- Write bended note frequency voltages to Note GPIOs
  analogWrite(2, voices[0].bendedNote);
  analogWrite(3, voices[1].bendedNote);
  analogWrite(4, voices[2].bendedNote);
  analogWrite(5, voices[3].bendedNote);
  analogWrite(6, voices[4].bendedNote);
  analogWrite(9, voices[5].bendedNote);
  analogWrite(22, voices[6].bendedNote);
  analogWrite(23, voices[7].bendedNote);

  //-------------------------- Fill Arpeggio buffer
  fillArpNotes();

  // --------------------- Write velocity voltages to velocity GPIOs
  analogWrite(10,veloVoltLin[voices[0].velocity]);
  analogWrite(11,veloVoltLin[voices[1].velocity]);
  analogWrite(12,veloVoltLin[voices[2].velocity]);
  analogWrite(13,veloVoltLin[voices[3].velocity]);
  analogWrite(14,veloVoltLin[voices[4].velocity]);
  analogWrite(15,veloVoltLin[voices[5].velocity]);
  analogWrite(18,veloVoltLin[voices[6].velocity]);
  analogWrite(19,veloVoltLin[voices[7].velocity]);
}