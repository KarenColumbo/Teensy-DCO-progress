#include <SPI.h>
#include "notes.h"

//Define pins
#define FSYNCPin 9
#define SDAPin 11
#define SCLKPin 13

//AD9833 register commands
#define RESET 0x0100
#define FREQ0 0x4000
#define FREQ1 0x8000
#define PHASE0 0xC000
#define PHASE1 0xE000
#define SIGN_PI 0x2000
#define OPBITEN 0x1000
#define SLEEP1 0x0080
#define SLEEP12 0x0100

//Used for changing AD9833 registers
unsigned int AD9833DATA;

void setFrequency(float freq) {
  // Calculate the frequency word
  uint32_t freqWord = freq * (pow(2, 28) / 25000000);

  // Split the frequency word into two bytes
  uint16_t MSB = (freqWord >> 14) & 0x3FFF;
  uint16_t LSB = freqWord & 0x3FFF;

  // Set control bits 15 and 14 to 0 and 1, respectively, for frequency register 0
  MSB |= FREQ0;

  // Set control bits 15 and 14 to 1 and 0, respectively, for frequency register 1
  MSB |= FREQ1;

  // Write to the AD9833
  digitalWrite(FSYNCPin, LOW);
  SPI.transfer16(MSB);
  SPI.transfer16(LSB);
  digitalWrite(FSYNCPin, HIGH);
  
  delay(100);  // Wait a short time to ensure FSYNC is low
}


void reset_AD9833() {
  //Reset word
  AD9833DATA = RESET;
  //Write reset word to AD9833
  digitalWrite(FSYNCPin, LOW);
  shiftOut(SDAPin, SCLKPin, MSBFIRST, (AD9833DATA >> 8));
  shiftOut(SDAPin, SCLKPin, MSBFIRST, (AD9833DATA & 0xFF));
  digitalWrite(FSYNCPin, HIGH);
}

void setup() {
  //Set pins as outputs
  pinMode(FSYNCPin, OUTPUT);
  pinMode(SDAPin, OUTPUT);
  pinMode(SCLKPin, OUTPUT);
  //Initialize SPI communication
  SPI.begin();
  SPI.setDataMode(SPI_MODE2);
  //Reset AD9833 to default values
  reset_AD9833();
}

void loop() {
  for (int i = 0; i <= 72; i++) {
    setFrequency(noteFrequency[i]);
    delay(500);
  }
}