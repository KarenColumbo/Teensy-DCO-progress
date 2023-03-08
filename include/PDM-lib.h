#ifndef PDM_LIB_H
#define PDM_LIB_H

#include <IntervalTimer.h>

class PDM {
  public:
    PDM(uint8_t pin, float voltage);
    void enable();
    void disable();
  private:
    static const uint32_t PDM_FS = 3072000; // PDM sample rate (adjust as needed)
    static const uint8_t PDM_BITS = 16; // PDM output bit depth (adjust as needed)
    static const uint32_t PDM_PERIOD = (uint32_t)(1000000.0 / PDM_FS); // PDM output period (in microseconds)
    static uint32_t pdmPulse; // PDM pulse width (in microseconds)
    static uint32_t pdmSilence; // PDM silence width (in microseconds)
    static uint8_t pdmPin; // PDM output pin
    static volatile bool pdmEnabled; // PDM output enable flag
    static volatile uint32_t pdmCounter; // PDM output counter
    static void outputPDMInterrupt();
};

void outputVoices();
void stopOutput();
float calculateVoltageFromFreq(float freq);

#endif
