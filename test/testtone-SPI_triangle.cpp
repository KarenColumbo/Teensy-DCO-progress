#include <SPI.h>
#include "notes.h"

// AD9833 control word and frequency register addresses
const uint16_t AD_CTRL = 0x2100U;
const uint16_t AD_FREQ0 = 0x4000U;
const uint16_t AD_FREQ1 = 0x8000U;

// Pins for SPI comm with the AD9833 IC
#define SCLK  13
#define SDATA 11

// FSYNC pins for 8 AD9833 chips
#define FSYNC_1 2
#define FSYNC_2 3
#define FSYNC_3 4
#define FSYNC_4 5
#define FSYNC_5 6
#define FSYNC_6 7
#define FSYNC_7 8
#define FSYNC_8 9

// AD9833 clock frequency (Hz)
#define F_MCLK 25000000

// Helper function to get the FSYNC pin for a given AD9833 chip index
uint8_t getFSYNCPin(int index) {
  const uint8_t FSYNC_PINS[] = {FSYNC_1, FSYNC_2, FSYNC_3, FSYNC_4, FSYNC_5, FSYNC_6, FSYNC_7, FSYNC_8};
  if ((unsigned int)index < sizeof(FSYNC_PINS) / sizeof(FSYNC_PINS[0])) {
    return FSYNC_PINS[index];
  } else {
    return 0;
  }
}

void setup() {
  // Set up the SPI bus
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE2);
  SPI.setBitOrder(MSBFIRST);

  // Set pin modes for FSYNC pins
  pinMode(FSYNC_1, OUTPUT);
  pinMode(FSYNC_2, OUTPUT);
  pinMode(FSYNC_3, OUTPUT);
  pinMode(FSYNC_4, OUTPUT);
  pinMode(FSYNC_5, OUTPUT);
  pinMode(FSYNC_6, OUTPUT);
  pinMode(FSYNC_7, OUTPUT);
  pinMode(FSYNC_8, OUTPUT);
}

void loop() {
  for (int i = 0; i < 73; i++)
  {
  float freq = noteFrequency[i];

  // Write frequency control word to all AD9833 chips
  for (int i = 0; i < 8; i++) {
    // Set FSYNC pin LOW for the current AD9833 chip
    digitalWriteFast(getFSYNCPin(i), LOW);

    // Write control register to set output to triangle, enable output, and reset phase
    SPI.transfer16(AD_CTRL | 0x02);

    // Calculate and write frequency control word
    uint32_t freqWord = freq * (pow(2, 28) / F_MCLK);
    uint16_t freqLow = freqWord & 0x3FFF;
    uint16_t freqHigh = (freqWord >> 14) & 0x3FFF;
    SPI.transfer16(AD_FREQ0 | freqLow);
    SPI.transfer16(AD_FREQ1 | freqHigh);

    // Set FSYNC pin HIGH for the current AD9833 chip
    digitalWriteFast(getFSYNCPin(i), HIGH);
  }
  }
}