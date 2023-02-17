#include <SPI.h>

#define FSYNC 9
#define CLK 13
#define MOSI 11

void initializeAD9833(float frequency) {
  // Calculate frequency register value
  unsigned long freqRegister = (unsigned long)(frequency * pow(2, 28) / 25000000);

  // Initialize SPI
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  // Set control register
  digitalWrite(FSYNC, LOW);
  SPI.transfer(0x2100);
  SPI.transfer(0x00);

  // Set frequency register 0
  digitalWrite(FSYNC, LOW);
  SPI.transfer(0x4000);
  SPI.transfer((freqRegister >> 14) & 0x3FFF);
  SPI.transfer(freqRegister & 0x3FFF);

  // Set phase register 0
  digitalWrite(FSYNC, LOW);
  SPI.transfer(0xC000);
  SPI.transfer(0x0000);

  // Exit reset
  digitalWrite(FSYNC, LOW);
  SPI.transfer(0x2000);
  SPI.transfer(0x00);

  // Set mode to sine wave
  digitalWrite(FSYNC, LOW);
  SPI.transfer(0x2100);
  SPI.transfer(0x02);

  // Deactivate FSYNC
  digitalWrite(FSYNC, HIGH);
}
