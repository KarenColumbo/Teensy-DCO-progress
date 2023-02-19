// DUAL CHANNEL
void AD9833setFrequency(int board, long frequency, int deformfactor) {
  long FreqReg0 = (frequency * pow(2, 28)) / MCLK;   // Data sheet Freq Calc formula
  long FreqReg1 = (frequency * pow(2, 28)) / MCLK;   // Data sheet Freq Calc formula
  int MSB0 = (int)((FreqReg0 & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
  int LSB0 = (int)(FreqReg0 & 0x3FFF);
  int MSB1 = (int)((FreqReg1 & 0xFFFC000) >> 14);     // only lower 14 bits are used for data
  int LSB1 = (int)(FreqReg1 & 0x3FFF);
  
  int FSYNC_SET_PIN = FSYNC_PINS[board];
  SPI.beginTransaction(SPISettings(SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE2));
  digitalWrite(FSYNC_SET_PIN, LOW);                         // set FSYNC low before writing to AD9833 registers

  LSB0 |= 0x4000;                                    // DB 15=0, DB14=1
  MSB0 |= 0x4000;                                    // DB 15=0, DB14=1
  LSB1 |= 0x4000;                                    // DB 15=0, DB14=1
  MSB1 |= 0x4000;                                    // DB 15=0, DB14=1
  SPI.transfer16(LSB0);                              // write lower 16 bits to AD9833 registers
  SPI.transfer16(MSB0);                              // write upper 16 bits to AD9833 registers
  SPI.transfer16(0xC000);                           // write phase register
  SPI.transfer16(0x2002);                           // take AD9833 out of reset and output triangle wave (DB8=0)

  SPI.transfer16(0x2000);                            // Select DAC B (DB13 = 1), Control bits = 0
  SPI.transfer16(LSB1);                              // write lower 16 bits to AD9833 registers
  SPI.transfer16(MSB1);                              // write upper 16 bits to AD9833 registers
  SPI.transfer16(0xC000);                           // write phase register
  SPI.transfer16(deformForm[deformfactor]);         // take AD9833 out of reset and output triangle wave (DB8=0)

  delayMicroseconds(2);                             // Settle time? Doesn't make much difference …

  digitalWrite(FSYNC_SET_PIN, HIGH);                        // write done, set FSYNC high
  SPI.endTransaction();
}
