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

#endif

/*
This implementation defines a PDM class with the following public methods:

    PDM(uint8_t pin, float voltage): constructor that takes the GPIO pin and output voltage as arguments and initializes the PDM parameters.
    void enable(): method that enables the PDM output on the specified GPIO pin.
    void disable(): method that disables the PDM output.

The private members of the PDM class include the PDM sample rate, output bit depth, output period, pulse width, silence width, output pin, enable flag, counter variable, and the interrupt service routine (outputPDMInterrupt) that is called by the IntervalTimer object to output the PDM signal on the specified GPIO pin.

To use the PDM library, you can simply include the PDM-Lib.h header file in your program and create a PDM object with the desired GPIO pin and output voltage. Then, you can call the enable method to start the PDM output and the disable method to stop the output when necessary.

Here's an example implementation of a program that uses the PDM library to output PDM signals for multiple voices with polyphony:
*/