#include <Wire.h>
#include "TCA9548.h"

// Define TCA9548A address
#define TCA_ADDR 0x70

// Define MCP4728 addresses
#define MCP_ADDR_1 0x60
#define MCP_ADDR_2 0x61
#define MCP_ADDR_3 0x62
#define MCP_ADDR_4 0x63

// Define function to write to MCP4728
void writeMCP4728(byte tcaChannel, byte mcpAddress, byte command, int data) {
  // Select TCA9548A channel
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << tcaChannel);
  Wire.endTransmission();

  // Write to MCP4728
  Wire.beginTransmission(mcpAddress);
  Wire.write(command);
  Wire.write(data >> 8);
  Wire.write(data & 0xFF);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
}

void loop() {
  // Write data to MCP4728 on channel 0 of TCA9548A
  writeMCP4728(0, MCP_ADDR_1, 0x40, 0x7FFF);

  // Write data to MCP4728 on channel 1 of TCA9548A
  writeMCP4728(1, MCP_ADDR_2, 0x40, 0x3FFF);

  // Write data to MCP4728 on channel 2 of TCA9548A
  writeMCP4728(2, MCP_ADDR_3, 0x40, 0xFFFF);

  // Write data to MCP4728 on channel 3 of TCA9548A
  writeMCP4728(3, MCP_ADDR_4, 0x40, 0x8000);

  delay(100);
}
