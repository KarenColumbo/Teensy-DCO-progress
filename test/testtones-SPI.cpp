#include <SPI.h>
//#include <MD_AD9833.h>
#include "notes.h"

// Pins for SPI comm with the AD9833 IC
#define DATA  11	///< SPI Data pin number
#define CLK   13	///< SPI Clock pin number
#define FSYNC 9	///< SPI Load pin number (FSYNC in AD9833 usage)

// AD9833 control word and frequency register addresses
const uint16_t AD_CTRL = 0x2000U;
const uint16_t AD_FREQ0 = 0x4000U;
const uint16_t AD_FREQ1 = 0x8000U;
const uint32_t FREQ_FACTOR = 268435456;
const uint32_t F_MCLK = 25000000;

//MD_AD9833	AD(FSYNC);

void adWrite(float frequency) {

  digitalWrite(CLK, LOW);
  digitalWrite(DATA, LOW);

  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE2));

  digitalWrite(FSYNC, LOW);
  
  SPI.transfer16(AD_CTRL);

  uint32_t freqWord = frequency * FREQ_FACTOR / F_MCLK;
  Serial.println(freqWord);
  uint16_t freqLow = freqWord & 0x3FFF;
  uint16_t freqHigh = (freqWord >> 14) & 0x3FFF;

  SPI.transfer16(AD_FREQ0 | freqLow);
  SPI.transfer16(AD_FREQ1 | freqHigh);
  
  

  digitalWrite(FSYNC, HIGH);
  
  SPI.endTransaction();
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  pinMode(DATA, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(FSYNC, OUTPUT);
  
	//AD.begin();
  //AD.setMode(MD_AD9833::MODE_TRIANGLE);
}

void loop () {
  for (int i = 0; i <= 72; i++) {
    //AD.setFrequency(MD_AD9833::CHAN_0, noteFrequency[i]);
    adWrite(noteFrequency[i]);
    Serial.println(noteFrequency[i]);
    delay(500);
  }
}
