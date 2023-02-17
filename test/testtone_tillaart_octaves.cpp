#include <SPI.h>
#include <MD_AD9833.h>
#include <MCP23S17.h>
#include "notes.h"
#include <SoftwareSerial.h>

#define MCP23S17_DATA 11
#define MCP23S17_CLK 13
#define MCP23S17_CS 10
#define FSYNC_0 0  // AD9833 #0 chip select
#define FSYNC_1 1  // AD9833 #1 chip select

MCP23S17 mcp(MCP23S17_CS);
MD_AD9833 AD0(FSYNC_0);
MD_AD9833 AD1(FSYNC_1);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mcp.begin();
  mcp.pinMode(FSYNC_0, OUTPUT);
  mcp.pinMode(FSYNC_1, OUTPUT);
  AD0.begin();
  AD0.setMode(MD_AD9833::MODE_TRIANGLE);
  AD1.begin();
  AD1.setMode(MD_AD9833::MODE_TRIANGLE);
}

void loop() {
  for (int i = 0; i <= 72; i++) {
    // Set frequency on AD9833 #0
    mcp.digitalWrite(FSYNC_0, LOW);
    AD0.setFrequency(MD_AD9833::CHAN_0, noteFrequency[i]);
    AD0.setFrequency(MD_AD9833::CHAN_1, noteFrequency[i] * 2);  // one octave higher
    mcp.digitalWrite(FSYNC_0, HIGH);

    // Set frequency on AD9833 #1
    mcp.digitalWrite(FSYNC_1, LOW);
    AD1.setFrequency(MD_AD9833::CHAN_0, noteFrequency[i]);
    AD1.setFrequency(MD_AD9833::CHAN_1, noteFrequency[i] * 2);  // one octave higher
    mcp.digitalWrite(FSYNC_1, HIGH);

    delay(500);
  }
}
