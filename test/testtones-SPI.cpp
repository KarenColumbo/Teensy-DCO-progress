#include <SPI.h>
#include "notes.h"
#include <SoftwareSerial.h>

// Pins for SPI comm with the AD9833 IC
#define DATA  11	///< SPI Data pin number
#define CLK   13	///< SPI Clock pin number
#define FSYNC 9	///< SPI Load pin number (FSYNC in AD9833 usage)
const uint32_t F_MCLK = 25000000;  // AD9833 clock frequency (Hz)

// AD9833 control word and frequency register addresses
const uint16_t AD_CTRL = 0x2100U | 0x02;
const uint16_t AD_FREQ0 = 0x4000U;
const uint16_t AD_FREQ1 = 0x8000U;
const uint32_t FREQ_FACTOR = 1UL << 28;

void setADFrequency(float frequency) {
  
  // Set FSYNC pin LOW
  digitalWrite(FSYNC, LOW);
  
  // Calculate and write frequency control words
  uint32_t freqWord = frequency * FREQ_FACTOR / F_MCLK;
  uint16_t freqLow = freqWord & 0x3FFF;
  uint16_t freqHigh = (freqWord >> 14) & 0x3FFF;

  // Write frequency control words to the AD9833
  SPI.transfer16(AD_FREQ0 | freqLow);
  SPI.transfer16(AD_FREQ1 | freqHigh);

  // Write control register to set output to SINE, enable output, and reset phase
  SPI.transfer16(AD_CTRL | 0x02);

  // Set FSYNC pin HIGH
  digitalWrite(FSYNC, HIGH);
}

void setup()
{
  Serial.begin(9600);
    
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE2);
  SPI.setBitOrder(MSBFIRST);
  
  // Set pin modes
  pinMode(CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(FSYNC, OUTPUT);
}

void loop()
{
  for (int i = 0; i <= 72; i++) 
  {
    setADFrequency(noteFrequency[i]);
    Serial.println(i);
    delay(1000);
  }  
}