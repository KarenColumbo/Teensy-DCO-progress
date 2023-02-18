void AD9833::begin() {
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE2));
  digitalWrite(_fsync_pin, HIGH); // ensure that FSYNC is initially high
  pinMode(_fsync_pin, OUTPUT);

  // Send reset command
  writeRegister(AD9833_RESET);

  // Set up registers
  writeRegister(AD9833_B28);
  writeRegister(AD9833_PHASE);
  writeRegister(AD9833_FREQ0);
  writeRegister(AD9833_FREQ0 + 2);
  writeRegister(AD9833_PHASE + 2);
  writeRegister(AD9833_CTRL);

  // Turn off reset
  writeRegister(AD9833_CTRL);
  SPI.endTransaction();
}