#include <Wire.h>
#include <Adafruit_MCP4728.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_TCA9548A.h>

#define DAC_ADDRESS1 0x60
#define DAC_ADDRESS2 0x61
#define DAC_ADDRESS3 0x62
#define DAC_ADDRESS4 0x63
#define DAC_ADDRESS5 0x64
#define MCP_ADDRESS 0x20

// Create an instance of the TCA9548A multiplexer
Adafruit_TCA9548A tca = Adafruit_TCA9548A();

// Create instances of the MCP4728 and MCP23X17 libraries
Adafruit_MCP4728 dac1 = Adafruit_MCP4728(DAC_ADDRESS1);
Adafruit_MCP4728 dac2 = Adafruit_MCP4728(DAC_ADDRESS2);
Adafruit_MCP4728 dac3 = Adafruit_MCP4728(DAC_ADDRESS3);
Adafruit_MCP4728 dac4 = Adafruit_MCP4728(DAC_ADDRESS4);
Adafruit_MCP4728 dac5 = Adafruit_MCP4728(DAC_ADDRESS5);
Adafruit_MCP23X17 mcp = Adafruit_MCP23X17(MCP_ADDRESS);

void setup() {
  // Start the I2C bus
  Wire.begin();

  // Initialize the TCA9548A multiplexer
  tca.begin(0x70);

  // Select the I2C channel on the TCA9548A for each board
  tca.setChannel(0); // Channel 0 for dac1
  dac1.begin();
  tca.setChannel(1); // Channel 1 for dac2
  dac2.begin();
  tca.setChannel(2); // Channel 2 for dac3
  dac3.begin();
  tca.setChannel(3); // Channel 3 for dac4
  dac4.begin();
  tca.setChannel(4); // Channel 4 for dac5
  dac5.begin();
  tca.setChannel(5); // Channel 5 for mcp
  mcp.begin();
}

void loop() {
  // Your code here...
}
