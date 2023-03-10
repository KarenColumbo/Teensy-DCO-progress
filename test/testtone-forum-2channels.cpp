#include <SPI.h> 
#include <SoftwareSerial.h>                     
#include "notes.h"

// pin 13 (SCK), pin 11 (MOSI), AD9833 generator
const int FSYNC = 9;                 // pin 10 (SS)
#define SPI_CLOCK_SPEED 12000000      // 12MHz SPI clock
unsigned long MCLK = 25000000;        // AD9833 onboard crystal reference frequency

void WriteRegister(int data) {
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  digitalWrite(FSYNC, LOW);           // set FSYNC low before writing to AD9833 registers
  SPI.transfer16(data);
  digitalWrite(FSYNC, HIGH);          // write done, set FSYNC high
  SPI.endTransaction();
}

void AD9833setFrequency(unsigned int channel, long frequency) {
  long FreqReg = (frequency * pow(2, 28)) / MCLK;
  int MSB = (int)((FreqReg & 0xFFFC000) >> 14);    // only lower 14 bits are used for data
  int LSB = (int)(FreqReg & 0x3FFF);

  if (channel == 0) {
    LSB |= 0x4000;                      // DB 15=0, DB14=1
    MSB |= 0x4000;                      // DB 15=0, DB14=1

    WriteRegister(0x2100);              // put AD9833 into reset and tell it to accept 14bit words (DB13=1, DB8=1)
    WriteRegister(LSB);                 // write lower 16 bits to AD9833 registers
    WriteRegister(MSB);                 // write upper 16 bits to AD9833 registers
    WriteRegister(0xC000);              // write phase register
    WriteRegister(0x2002);              // take AD9833 out of reset and output sinewave (DB8=0)
  }
  else if (channel == 1) {
    LSB |= 0xC000;                      // DB 15=1, DB14=1
    MSB |= 0xC000;                      // DB 15=1, DB14=1

    WriteRegister(0x2102);              // put AD9833 into reset and tell it to accept 14bit words (DB13=1, DB9=1)
    WriteRegister(LSB);                 // write lower 16 bits to AD9833 registers
    WriteRegister(MSB);                 // write upper 16 bits to AD9833 registers
    WriteRegister(0xE000);              // write phase register
    WriteRegister(0x2028);              // take AD9833 out of reset and output sinewave (DB9=0)
  }
  
}

void setup() {
  pinMode(FSYNC, OUTPUT);
  digitalWrite(FSYNC, HIGH); 
  SPI.begin();
  AD9833setFrequency(0, 1000);        // set frequency for channel 0
  AD9833setFrequency(1, 2000);        // set frequency for channel 1
}

void loop () {
  for (int i = 0; i <= 72; i++) {
    
    AD9833setFrequency(0, noteFrequency[i]);
    //AD9833setFrequency(1, noteFrequency[i]);
    delay(500);
  }
}
