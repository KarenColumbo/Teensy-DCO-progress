#include <SPI.h>
#include <MD_AD9833.h>
#include <MCP23S17.h>
#include "notes.h"
#include <SoftwareSerial.h>

#define MCP23S17_DATA  11
#define MCP23S17_CLK   13
#define MCP23S17_CS    10
#define FSYNC 1

MCP23S17 mcp(MCP23S17_CS);
MD_AD9833 AD(FSYNC);

void setup(void)
{
  Serial.begin(115200);
  SPI.begin();
  mcp.begin();
  mcp.pinMode(FSYNC, OUTPUT);
  AD.begin();
  AD.setMode(MD_AD9833::MODE_TRIANGLE);
}

void loop (void)
{
  for (int i = 0; i <= 72; i++) 
  {
    Serial.print("Setting frequency to ");
    Serial.println(noteFrequency[i]);
    AD.setFrequency(MD_AD9833::CHAN_0, noteFrequency[i]);
    delay(500);
  }
}
