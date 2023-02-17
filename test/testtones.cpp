#include <SPI.h>
#include <MD_AD9833.h>
#include "notes.h"

// Pins for SPI comm with the AD9833 IC
#define DATA  11	///< SPI Data pin number
#define CLK   13	///< SPI Clock pin number
#define FSYNC 9	///< SPI Load pin number (FSYNC in AD9833 usage)

MD_AD9833	AD(FSYNC);

void setup(void)
{
  
	AD.begin();
  AD.setMode(MD_AD9833::MODE_TRIANGLE);
}

void loop (void)
{
  for (int i = 45; i <= 72; i++) 
  {
    AD.setFrequency(MD_AD9833::CHAN_0, noteFrequency[i]);
    delay(500);
  }
}
