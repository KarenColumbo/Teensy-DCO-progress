#include <SPI.h
#include <notes.h>

// AD9833 control word and frequency register addresses
const uint16_t AD_CTRL = 0x2100U;
const uint16_t AD_FREQ0 = 0x4000U;
const uint16_t AD_FREQ1 = 0x8000U;

// Pins for SPI comm with the AD9833 IC
#define SCLK  13
#define SDATA 11
#define FSYNC 10

// AD9833 clock frequency (Hz)
#define F_MCLK 25000000

void setup() {
  // Set up the SPI bus
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE2);
  SPI.setBitOrder(MSBFIRST);

  // Set pin modes
  pinMode(SCLK, OUTPUT);
  pinMode(SDATA, OUTPUT);
  pinMode(FSYNC, OUTPUT);
}

void loop() {
  // Set frequency to 440 Hz (A4)
  for (int i = 0; i <= 73; i++)
  {
  float freq = noteFrequency[i];
  
  // Set FSYNC pin LOW
  digitalWriteFast(FSYNC, LOW);

  // Write control register to set output to SINE, enable output, and reset phase
  SPI.transfer16(AD_CTRL);

  // Calculate and write frequency control word
  uint32_t freqWord = freq * (pow(2, 28) / F_MCLK);
  uint16_t freqLow = freqWord & 0x3FFF;
  uint16_t freqHigh = (freqWord >> 14) & 0x3FFF;
  SPI.transfer16(AD_FREQ0 | freqLow);
  SPI.transfer16(AD_FREQ1 | freqHigh);

  // Set FSYNC pin
  digitalWriteFast(FSYNC, LOW);

  // Write control register to set output to SINE, enable output, and reset phase
  SPI.transfer16(AD_CTRL);

  // Calculate and write frequency control word
  uint32_t freqWord = freq * (pow(2, 28) / F_MCLK);
  uint16_t freqLow = freqWord & 0x3FFF;
  uint16_t freqHigh = (freqWord >> 14) & 0x3FFF;
  SPI.transfer16(AD_FREQ0 | freqLow);
  SPI.transfer16(AD_FREQ1 | freqHigh);

  // Set FSYNC pin HIGH
  digitalWriteFast(FSYNC, HIGH);
  
  // Wait a bit before setting a new frequency
  delay(10);
  }
}
