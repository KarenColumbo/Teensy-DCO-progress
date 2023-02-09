#include <Wire.h>

const int s0 = 13; // Pin 13 connected to S0 on the 74HC4067
const int s1 = 12; // Pin 12 connected to S1 on the 74HC4067
const int s2 = 11; // Pin 11 connected to S2 on the 74HC4067
const int s3 = 10; // Pin 10 connected to S3 on the 74HC4067

void setup() {
  // Initialize the I2C communication library
  Wire.begin();

  // Set the address pins as outputs
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
}

void setAddress(int address) {
  // Set the address pins to select the desired MCP4728 board
  digitalWrite(s0, (address & 1) == 1);
  digitalWrite(s1, (address & 2) == 2);
  digitalWrite(s2, (address & 4) == 4);
  digitalWrite(s3, (address & 8) == 8);
}

void writeToMCP4728(int address, int value) {
  // Select the desired MCP4728 board
  setAddress(address);

  // Write the value to the MCP4728
  Wire.beginTransmission(0x60); // MCP4728 I2C address
  Wire.write(0x40); // Write to the DAC register
  Wire.write(value >> 8); // High byte of value
  Wire.write(value & 0xff); // Low byte of value
  Wire.endTransmission();
}

void loop() {
  // Write a value to each MCP4728 board
  writeToMCP4728(0, 512); // Write 512 to the first MCP4728
  writeToMCP4728(1, 256); // Write 256 to the second MCP4728
  writeToMCP4728(2, 128); // Write 128 to the third MCP4728

  // Wait for a moment before writing new values
  delay(1000);
}
/*

Yes, a 74HC4067 16-channel analog multiplexer/demultiplexer could be used to address multiple MCP4728 boards. This device allows you to select one of 16 channels, each of which can be connected to a separate device, such as an MCP4728.

Here's a basic overview of how you could set this up:

    Connect the SDA and SCL lines from each MCP4728 board to the common SDA and SCL inputs on the 74HC4067.

    Connect the Teensy 4.1's SDA1 and SCL1 pins to the SDA and SCL outputs on the 74HC4067.

    Connect the address pins (S0, S1, S2, S3) on the 74HC4067 to the appropriate voltage levels to select the desired MCP4728 board.

    In your Teensy 4.1 program, use the 74HC4067 to select the desired MCP4728 board by setting the appropriate address pins.

    Communicate with the selected MCP4728 board as normal, using the I2C communication functions in the Teensy 4.1 libraries.

This setup will allow you to communicate with each of the MCP4728 boards using a single set of SDA and SCL pins on the Teensy 4.1, while still allowing you to select which board to communicate with by setting the appropriate address pins on the 74HC4067.
*/