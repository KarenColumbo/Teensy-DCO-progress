#include <SPI.h>
#include "notes.h"
#include <Adafruit_MCP23X17.h>

// AD9833 control word and frequency register addresses
const uint16_t AD_CTRL = 0x2100U;

// Define the MCP23S17 object
Adafruit_MCP23X17 mcp;

// Define the AD9833 frequency control commands
const uint16_t AD_FREQ0 = 0x2100; // FREQ0 address + control bits
const uint16_t AD_FREQ1 = 0x2300; // FREQ1 address + control bits

// AD9833 clock frequency (Hz)
#define F_MCLK 25000000

void setup() {
  // Initialize the MCP23S17
  mcp.begin_SPI(10);
  mcp.pinMode(11, INPUT); // set the CS pin as an input
  mcp.pinMode(21, OUTPUT); // set GPIOA0 as an ouput
  //mcp.pinMode(22, OUTPUT); // set GPIOA1 as an output
  //mcp.pinMode(23, OUTPUT); // set GPIOA2 as an output
  //mcp.pinMode(24, OUTPUT); // set GPIOA3 as an output
  //mcp.pinMode(25, OUTPUT); // set GPIOA4 as an output
  //mcp.pinMode(26, OUTPUT); // set GPIOA5 as an output
  //mcp.pinMode(27, OUTPUT); // set GPIOA6 as an output
  //mcp.pinMode(28, OUTPUT); // set GPIOA7 as an output
}

void loop() {
  for (int i = 0; i < 73; i++)
  {
    float freq = noteFrequency[i];

    // Pull FSYNC pin LOW
    mcp.digitalWrite(21, LOW);

    // Write control register to set output to triangle, enable output, and reset phase
    SPI.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE2));
    SPI.transfer16(AD_CTRL | 0x02);
    SPI.endTransaction();

    // Calculate and write frequency control word
    uint32_t freqWord = freq * (pow(2, 28) / F_MCLK);
    uint16_t freqLow = freqWord & 0x3FFF;
    uint16_t freqHigh = (freqWord >> 14) & 0x3FFF;

    SPI.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE2));
    SPI.transfer16(AD_FREQ0 | freqLow);
    SPI.transfer16(AD_FREQ1 | freqHigh);
    SPI.endTransaction();

    // Set FSYNC pin HIGH for the current AD9833 chip
    mcp.digitalWrite(21, HIGH);
    delay(1000);
  }
}