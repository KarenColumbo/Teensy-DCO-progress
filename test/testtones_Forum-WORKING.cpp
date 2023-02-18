#include <SPI.h> 
#include "notes.h"

// Teensy 4.1 pin 13 (SCK)  --->  AD9833 board SCLK
// Teensy 4.1 pin 11 (MOSI) --->  AD9833 board SDATA
// Teensy 4.1 pin 9 (GPIO)  --->  Ad9833 board FSYNC

const int FSYNC = 9;                 
#define SPI_CLOCK_SPEED 7500000                     // 7.5 MHz SPI clock - this works ALMOST without clock ticks
unsigned long MCLK = 25000000;                      // AD9833 board default reference frequency

void AD9833setFrequency(long frequency) {
  long FreqReg = (frequency * pow(2, 28)) / MCLK;   // Data sheet Freq Calc formula
  int MSB = (int)((FreqReg & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
  int LSB = (int)(FreqReg & 0x3FFF);
  
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  digitalWrite(FSYNC, LOW);                         // set FSYNC low before writing to AD9833 registers

  LSB |= 0x4000;                                    // DB 15=0, DB14=1
  MSB |= 0x4000;                                    // DB 15=0, DB14=1
  SPI.transfer16(LSB);                              // write lower 16 bits to AD9833 registers
  SPI.transfer16(MSB);                              // write upper 16 bits to AD9833 registers
  SPI.transfer16(0xC000);                           // write phase register
  SPI.transfer16(0x2002);                           // take AD9833 out of reset and output triangle wave (DB8=0)
  delayMicroseconds(2);                             // Settle time? Doesn't make much difference …

  digitalWrite(FSYNC, HIGH);                        // write done, set FSYNC high
  SPI.endTransaction();
}

void setup() {
  pinMode(FSYNC, OUTPUT);                           // Prepare FSYNC pin for output
  digitalWrite(FSYNC, HIGH);                        // Set it high for good measure
  SPI.begin();
  SPI.transfer16(0x2100);                           // put AD9833 into reset and tell it to accept 14bit words (DB13=1, DB8=1) once
}

void loop () {
  for (int i = 0; i <= 72; i++) {                   // Read MIDI note array ("notes.h", C2–C7)
    AD9833setFrequency(noteFrequency[i]);           // Go play!
    delay(250);
  }
}
