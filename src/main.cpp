#include <SPI.h>                      // pin 13 (SCK), pin 11 (MOSI), AD9833 generator
const int FSYNC = 10;                 // pin 10 (SS)
#define SPI_CLOCK_SPEED 12000000      // 12MHz SPI clock
unsigned long MCLK = 25000000;        // AD9833 onboard crystal reference frequency
unsigned long freq = 1000;            // set initial frequency




void WriteRegister(int data) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  digitalWrite(FSYNC, LOW);           // set FSYNC low before writing to AD9833 registers
  SPI.transfer16(data);
  digitalWrite(FSYNC, HIGH);          // write done, set FSYNC high
  SPI.endTransaction();
}
void AD9833setFrequency(long frequency) {
  long FreqReg = (frequency * pow(2, 28)) / MCLK;
  int MSB = (int)((FreqReg & 0xFFFC000) >> 14);    // only lower 14 bits are used for data
  int LSB = (int)(FreqReg & 0x3FFF);

  LSB |= 0x4000;                      // DB 15=0, DB14=1
  MSB |= 0x4000;                      // DB 15=0, DB14=1

  WriteRegister(0x2100);              // put AD9833 into reset and tell it to accept 14bit words (DB13=1, DB8=1)
  WriteRegister(LSB);                 // write lower 16 bits to AD9833 registers
  WriteRegister(MSB);                 // write upper 16 bits to AD9833 registers
  WriteRegister(0xC000);              // write phase register
  WriteRegister(0x2000);              // take AD9833 out of reset and output sinewave (DB8=0)
}
void setup() {
  pinMode (FSYNC, OUTPUT);
  digitalWrite(FSYNC, HIGH); 
  SPI.begin();
  AD9833setFrequency(freq);           // set frequency
}

void loop() {
}


