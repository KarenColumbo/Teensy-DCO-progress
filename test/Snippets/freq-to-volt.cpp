// Define the input frequency range (C2 to C7 with 4 decimal precision)
const float MIN_FREQ = 65.4064; // C2 frequency
const float MAX_FREQ = 2093.0045; // C7 frequency

// Define the output DAC range (0 to 4095)
const int MIN_DAC = 0;
const int MAX_DAC = 4095;

// Convert a float frequency to an int DAC value
int freqToDAC(float freq) {
  // Map the input frequency range to the output DAC range
  int dac = map(freq, MIN_FREQ, MAX_FREQ, MIN_DAC, MAX_DAC);
  return dac;
}

---------------

float input = 440.0; // Example input frequency in Hz
int outputMin = 0;
int outputMax = 4095;

float inputMin = 65.41; // MIDI note C2
float inputMax = 2093.00; // MIDI note C7

// Calculate scaling factor
float scale = (outputMax - outputMin) / (inputMax - inputMin);

// Map the input value to the output value
int output = (int)((input - inputMin) * scale + outputMin);
