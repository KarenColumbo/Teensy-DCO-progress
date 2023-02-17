#include <SPI.h>

#define FSYNC 9
#define CLK 13
#define MOSI_PIN 11
#define SYSTEM_CLOCK_FREQ 25000000UL // Set your system clock frequency here
#define RESET (uint16_t)0x0100


void initializeAD9833(float frequency)
{
  // Set the FSYNC pin as output
  pinMode(FSYNC, OUTPUT);

  // Initialize SPI
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  // Reset the AD9833
  digitalWrite(FSYNC, LOW);
  SPI.transfer(RESET);
  digitalWrite(FSYNC, HIGH);

  // Write to control register
  digitalWrite(FSYNC, LOW);
  SPI.transfer32((uint32_t)0x21000000);
  digitalWrite(FSYNC, HIGH);

  // Write to frequency register 0
  uint32_t frequencyWord = (uint32_t)(frequency * pow(2, 28) / SYSTEM_CLOCK_FREQ);
  uint16_t frequencyLSB = frequencyWord & 0x3FFF;
  uint16_t frequencyMSB = (frequencyWord >> 14) & 0x3FFF;
  uint32_t frequencyData = (0x40000000 | (uint32_t)frequencyLSB | ((uint32_t)frequencyMSB << 16));
  digitalWrite(FSYNC, LOW);
  SPI.transfer32(frequencyData);
  digitalWrite(FSYNC, HIGH);

  // Write to phase register 0
  digitalWrite(FSYNC, LOW);
  SPI.transfer32((uint32_t)0xC0000000);
  digitalWrite(FSYNC, HIGH);

  // Exit reset
  digitalWrite(FSYNC, LOW);
  SPI.transfer32((uint32_t)0x20000000);
  digitalWrite(FSYNC, HIGH);
}

void setup() {
  // Set pins to output
  pinMode(FSYNC, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);

  // Start SPI
  SPI.begin();

  // Set SPI clock speed to 1 MHz
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  // Set AD9833 to generate a 400 Hz signal with a 25 MHz MCLK
  initializeAD9833(400.0);
}

void loop() {
  // Nothing to do in the loop
}
