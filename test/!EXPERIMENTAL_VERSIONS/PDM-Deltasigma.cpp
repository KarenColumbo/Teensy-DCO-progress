float calculateVoltageFromFreq(uint16_t freq) {
  float voltage = ((float)freq / 16383.0) * 3.3; // assuming 3.3V reference voltage
  return voltage;
}

// --------------- PDM
void outputVoltageAsPDM(uint8_t pin, float voltage) {
  const uint32_t PDM_FS = 3072000; // PDM sample rate (adjust as needed)
  const uint8_t PDM_BITS = 16; // PDM output bit depth (adjust as needed)
  const uint32_t PDM_PERIOD = (uint32_t)(1000000.0 / PDM_FS); // PDM output period (in microseconds)
  const uint32_t PDM_PULSE = (uint32_t)(PDM_PERIOD * voltage / 3.3); // PDM pulse width (in microseconds)
  const uint32_t PDM_SILENCE = PDM_PERIOD - PDM_PULSE; // PDM silence width (in microseconds)
  
  digitalWriteFast(pin, LOW); // set initial state to LOW
  delayMicroseconds(PDM_PULSE); // output PDM pulse width
  digitalWriteFast(pin, HIGH); // set state to HIGH
  delayMicroseconds(PDM_SILENCE); // output PDM silence width
}

// ------------ With interrupts
volatile bool pdmEnabled = false;
volatile uint32_t pdmPeriod = 0;
volatile uint32_t pdmPulse = 0;
volatile uint32_t pdmSilence = 0;
volatile uint32_t pdmCounter = 0;

void outputPDMInterrupt() {
  if (pdmEnabled) {
    if (pdmCounter < pdmPulse) {
      digitalWriteFast(pin, LOW); // set output pin to LOW during pulse
    } else if (pdmCounter < pdmPeriod) {
      digitalWriteFast(pin, HIGH); // set output pin to HIGH during silence
    } else {
      pdmCounter = 0; // reset counter at end of period
    }
    pdmCounter++; // increment counter
  }
}

void enablePDM(uint8_t pin, float voltage) {
  const uint32_t PDM_FS = 3072000; // PDM sample rate (adjust as needed)
  const uint8_t PDM_BITS = 16; // PDM output bit depth (adjust as needed)
  pdmPeriod = (uint32_t)(1000000.0 / PDM_FS); // PDM output period (in microseconds)
  pdmPulse = (uint32_t)(pdmPeriod * voltage / 3.3); // PDM pulse width (in microseconds)
  pdmSilence = pdmPeriod - pdmPulse; // PDM silence width (in microseconds)
  
  // configure timer interrupt
  IntervalTimer pdmTimer;
  pdmTimer.begin(outputPDMInterrupt, pdmPeriod);
  
  // enable PDM output
  pdmEnabled = true;
}

void disablePDM() {
  // disable PDM output
  pdmEnabled = false;
}

// ---------------- Call Routines
const uint8_t PDM_PINS[POLYPHONY] = {13, 14, 15}; // output pins for PDM signals
volatile bool pdmEnabled = false;
volatile uint32_t pdmPeriod = 0;
volatile uint32_t pdmPulse = 0;
volatile uint32_t pdmSilence = 0;
volatile uint32_t pdmCounters[POLYPHONY] = {0};

void outputPDMInterrupt() {
  if (pdmEnabled) {
    for (int i = 0; i < POLYPHONY; i++) {
      if (pdmCounters[i] < pdmPulse) {
        digitalWriteFast(PDM_PINS[i], LOW); // set output pin to LOW during pulse
      } else if (pdmCounters[i] < pdmPeriod) {
        digitalWriteFast(PDM_PINS[i], HIGH); // set output pin to HIGH during silence
      } else {
        pdmCounters[i] = 0; // reset counter at end of period
      }
      pdmCounters[i]++; // increment counter
    }
  }
}

void enablePDM(uint8_t voice, float voltage) {
  const uint32_t PDM_FS = 3072000; // PDM sample rate (adjust as needed)
  const uint8_t PDM_BITS = 16; // PDM output bit depth (adjust as needed)
  pdmPeriod = (uint32_t)(1000000.0 / PDM_FS); // PDM output period (in microseconds)
  pdmPulse = (uint32_t)(pdmPeriod * voltage / 3.3); // PDM pulse width (in microseconds)
  pdmSilence = pdmPeriod - pdmPulse; // PDM silence width (in microseconds)
  
  // configure timer interrupt if PDM is not already enabled
  if (!pdmEnabled) {
    IntervalTimer pdmTimer;
    pdmTimer.begin(outputPDMInterrupt, pdmPeriod);
    pdmEnabled = true;
  }
  
  // reset counter for specified voice
  pdmCounters[voice] = 0;
}

void disablePDM() {
  // disable PDM output
  pdmEnabled = false;
}

void outputVoices() {
  for (int i = 0; i < POLYPHONY; i++) {
float voltage = calculateVoltageFromFreq(voices[i].dcoFreq);
enablePDM(i, voltage);
}
}
 /*
 
This implementation calls the `enablePDM` function for each voice with the voice index `i` and the calculated voltage, which will reset the corresponding counter variable and start outputting the PDM signal for the voice on the specified GPIO pin.

To stop the PDM output, you can simply call the `disablePDM` function. Note that you might need to adjust the PDM sample rate and output bit depth, as well as the PDM output pin and interval timer configuration, depending on your specific application and hardware requirements.

    RC low-pass filter:
    For a simple RC filter, you can use a value of 1kΩ for the resistor and a value of 10nF for the capacitor. The cutoff frequency of the filter will be approximately 15.9kHz, which is well below the 3.072MHz PDM sample rate. The filter transfer function is given by:

    H(s) = 1 / (1 + sRC)

    where s is the Laplace variable. You can implement this filter using a single resistor and capacitor in series with the output pin.

    Sallen-Key low-pass filter:
    For a more advanced filter, you can use a Sallen-Key topology with an op-amp. Here's an example filter with a cutoff frequency of 10kHz:

    Sallen-Key Filter Diagram

    The filter transfer function is given by:

    H(s) = 1 / (1 + sR1C1 + s^2R1R2C1C2 + sR2C2)

    where s is the Laplace variable. You can calculate the resistor and capacitor values as follows:

    R1 = R2 = 10kΩ
    C1 = C2 = 10nF

    The cutoff frequency of the filter will be approximately 10kHz, which is well below the 3.072MHz PDM sample rate. You can implement this filter using an op-amp, such as the LM358 or TL072, with the filter components connected as shown in the diagram.

Note that the exact filter requirements may vary depending on the specific analog part you are controlling, as well as the characteristics of the PDM output signal. You may need to adjust the filter parameters, such as the resistor and capacitor values, to achieve the desired level of filtering and performance. Additionally, you may need to add a buffer or amplifier stage after the filter to ensure sufficient signal levels and impedance matching.

*/

#include "PDM-Lib.h"

const uint8_t PDM_PINS[POLYPHONY] = {13, 14, 15}; // output pins for PDM signals
PDM pdmVoices[POLYPHONY];

void outputVoices() {
  for (int i = 0; i < POLYPHONY; i++) {
    float voltage = calculateVoltageFromFreq(voices[i].dcoFreq);
    pdmVoices[i] = PDM(PDM_PINS[i], voltage);
    pdmVoices[i].enable();
  }
}

void stopOutput() {
  for (int i = 0; i < POLYPHONY; i++) {
    pdmVoices[i].disable();
  }
}

void setup() {
  // initialize PDM output pins and voices
  outputVoices();
}

void loop() {
  // do something
}

