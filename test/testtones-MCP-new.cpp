#include <SPI.h> 
#include "notes.h"
#include <Adafruit_MCP23X17.h>

// WIRING

// Teensy 4.1 pin 13 (SCK)  --->  MCP23S17 pin 12 (SCK)   --->    AD9833 board SCLK
// Teensy 4.1 pin 11 (MOSI) --->  MCP23S17 pin 13 (SI)    --->    AD9833 board SDATA
// Teensy 4.1 pin 10 (CS)   --->  MCP23S17 pin 11 (CS)
// MCP23S17 pin 21 (GPA0)   --->  AD9833 board FSYNC

const int MCP_FSYNC_PIN = 0;                        //MCP23S17 FSYNC PIN = GPA0, pin 21
// const int FSYNC = 9;                             // Teensy 4.1 FSYNC pin
const int CS_PIN = 10;                              // Teensy 4.1 CS pin
#define SPI_CLOCK_SPEED 7500000                    // 7.5 MHz SPI clock - this works ALMOST without clock ticks
unsigned long MCLK = 25000000;                      // AD9833 board default reference frequency

Adafruit_MCP23X17 mcp;                              // Generate MCP23S17 object

void AD9833setFrequency(long frequency) {
  long FreqReg = (frequency * pow(2, 28)) / MCLK;   // Data sheet Freq Calc formula
  int MSB = (int)((FreqReg & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
  int LSB = (int)(FreqReg & 0x3FFF);
  
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  mcp.digitalWrite(MCP_FSYNC_PIN, LOW);             // set FSYNC low before writing to AD9833 registers

  LSB |= 0x4000;                                    // DB 15=0, DB14=1
  MSB |= 0x4000;                                    // DB 15=0, DB14=1
  SPI.transfer16(LSB);                              // write lower 16 bits to AD9833 registers
  SPI.transfer16(MSB);                              // write upper 16 bits to AD9833 registers
  SPI.transfer16(0xC000);                           // write phase register
  SPI.transfer16(0x2002);                           // take AD9833 out of reset and output triangle wave (DB8=0)
  delayMicroseconds(20);                            // Settle time? Doesn't make much difference …

  mcp.digitalWrite(MCP_FSYNC_PIN, HIGH);            // write done, set FSYNC high
  SPI.endTransaction();
}

void setup() {
  SPI.begin();                                      // Initialize SPI
  mcp.begin_SPI(CS_PIN);                            // Initialize MCP23S17
  mcp.pinMode(MCP_FSYNC_PIN, OUTPUT);               // Prepare MCP23S17 FSYNC pin for output
  //mcp.digitalWrite(MCP_FSYNC_PIN, HIGH);            // Set it high for good measure
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  mcp.digitalWrite(MCP_FSYNC_PIN, LOW);
  SPI.transfer16(0x2100);                           // put AD9833 into reset and tell it to accept 14bit words (DB13=1, DB8=1) once
  mcp.digitalWrite(MCP_FSYNC_PIN, HIGH);
}

void loop () {
  for (int i = 0; i <= 72; i++) {                   // Read MIDI note array ("notes.h", C2–C7)
    AD9833setFrequency(noteFrequency[i]);           // Go play!
    delay(250);
  }
}
