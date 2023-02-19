#include "Arduino.h"
#include <Wire.h>
#include "TCA9548.h"

// Define TCA9548A address
#define TCA_ADDR 0x70

// Define MCP4728 command byte
#define MCP_CMD 0x40

// Define function to write to MCP4728
void writeMCP4728(byte tcaChannel, byte mcpChannel, int data) {
  // Select TCA9548A channel
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << tcaChannel);
  Wire.endTransmission();

  // Write to MCP4728
  Wire.beginTransmission(0x60 | mcpChannel);
  Wire.write(MCP_CMD);
  Wire.write(data >> 8);
  Wire.write(data & 0xFF);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  Wire.setWireTimeout(1000); // Set a timeout for I2C transactions
  Wire.setClock(400000); // Set the I2C clock frequency to 400 kHz
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(0x00); // Turn on internal pullup resistors
  Wire.endTransmission();
}

void loop() {
  // Write data to MCP4728 channel 0 on TCA9548A channel 0
  writeMCP4728(0, 0, 0x7FFF);

  // Write data to MCP4728 channel 1 on TCA9548A channel 0
  writeMCP4728(0, 1, 0x3FFF);

  // Write data to MCP4728 channel 2 on TCA9548A channel 0
  writeMCP4728(0, 2, 0xFFFF);

  // Write data to MCP4728 channel 3 on TCA9548A channel 0
  writeMCP4728(0, 3, 0x8000);

  delay(100);
}
