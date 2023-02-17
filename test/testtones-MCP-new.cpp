#include <SPI.h>
#include <MD_AD9833.h>
#include <Adafruit_MCP23X17.h>
#include "notes.h"
#include <SoftwareSerial.h>

#define MCP23S17_DATA  11
#define MCP23S17_CLK   13
#define MCP23S17_CS    10
#define FSYNC 1

Adafruit_MCP23X17 mcp;
MD_AD9833 AD(FSYNC);

void setup(void)
{
  Serial.begin(115200);
  mcp.begin_SPI(MCP23S17_CS);
  mcp.pinMode(FSYNC, OUTPUT);
  AD.begin();
  AD.setMode(MD_AD9833::MODE_TRIANGLE);
  AD.setFrequencyClock(1000000); // set the clock frequency to 1 MHz
}

void loop (void)
{
  for (int i = 0; i <= 72; i++) 
  {
    Serial.print("Setting frequency to ");
    Serial.println(noteFrequency[i]);
    mcp.digitalWrite(FSYNC, LOW);
    AD.setFrequency(MD_AD9833::CHAN_0, noteFrequency[i]);
    delayMicroseconds(100);
    mcp.digitalWrite(FSYNC, HIGH);
    delay(500);
  }
}
