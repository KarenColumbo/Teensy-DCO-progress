#include <EEPROM.h>
#include <Bounce2.h>

const int SAVE_SWITCH_PIN = 2;
const int LOAD_SWITCH_PIN = 3;
const int EEPROM_ADDRESS = 0;
const int DEBOUNCE_DELAY = 20;
const int THRESHOLD = 3000;

Bounce saveSwitch = Bounce();
Bounce loadSwitch = Bounce();
unsigned long startTime = 0;
bool saveInProgress = false;
bool loadInProgress = false;

void setup() {
  // ...
  pinMode(SAVE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(LOAD_SWITCH_PIN, INPUT_PULLUP);
  saveSwitch.attach(SAVE_SWITCH_PIN);
  loadSwitch.attach(LOAD_SWITCH_PIN);
  saveSwitch.interval(DEBOUNCE_DELAY);
  loadSwitch.interval(DEBOUNCE_DELAY);
}

void loop() {
  // ...

  // Read MIDI controller 70-79, write to DACs
  if (MIDI.getType() == midi::ControlChange && MIDI.getChannel() == MIDI_CHANNEL) {
    uint8_t ccNumber = MIDI.getData1();
    uint8_t ccValue = MIDI.getData2();
    switch (ccNumber) {
      case 70:
        tcaselect(0);
        dac_0.setChannelValue(MCP4728_CHANNEL_C, map(ccValue, 0, 127, 0, 4095));
        break;
      case 71:
        tcaselect(0);
        dac_0.setChannelValue(MCP4728_CHANNEL_D, map(ccValue, 0, 127, 0, 4095));
        break;
      // ...
    }
  }

  // Check if the save switch is pressed
  saveSwitch.update();
  if (saveSwitch.fallingEdge() && !loadInProgress) {
    startTime = millis();
    saveInProgress = true;
  }

  // Check if the load switch is pressed
  loadSwitch.update();
  if (loadSwitch.fallingEdge() && !saveInProgress) {
    startTime = millis();
    loadInProgress = true;
  }

  // Save values to NVM if the save switch has been pressed for more than THRESHOLD milliseconds
  if (saveInProgress && (millis() - startTime >= THRESHOLD)) {
    // Save values
    EEPROM.write(EEPROM_ADDRESS, dac_0.getChannelValue(MCP4728_CHANNEL_C));
    EEPROM.write(EEPROM_ADDRESS + 1, dac_0.getChannelValue(MCP4728_CHANNEL_D));
    // ...
    saveInProgress = false;
  }

  // Load values from NVM if the load switch has been pressed for more than THRESHOLD milliseconds
  if (loadInProgress && (millis() - startTime >= THRESHOLD)) {
    // Load values
    uint16_t value;
    value = EEPROM.read(EEPROM_ADDRESS);
    dac_0.setChannelValue(MCP4728_CHANNEL_C, value);
    value = EEPROM.read(EEPROM_ADDRESS + 1);
    dac_0.setChannelValue(MCP4728_CHANNEL_D, value);
    // ...
    loadInProgress = false;
  }
}

/*
In this example, the `saveInProgress` and `loadInProgress` variables are used to keep track of whether a save or load operation is in progress, respectively. If either switch is pressed, the corresponding operation will not start if the other operation is already in progress. If either switch has been pressed for more than `THRESHOLD` milliseconds, the corresponding operation will be performed.
*/