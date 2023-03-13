const int NUM_CHANNELS = 4; // number of channels on MCP4728
int previous_voltage[NUM_CHANNELS]; // array to store previous voltage levels
bool dirty_flag[NUM_CHANNELS]; // array to store dirty flag for each channel

// function to update the voltage level for a particular channel
void update_voltage(int channel, int voltage) {
  // send command to MCP4728 to update voltage level for the specified channel
  // ...
}

void setup() {
  // initialize previous voltage and dirty flag arrays
  for (int i = 0; i < NUM_CHANNELS; i++) {
    previous_voltage[i] = 0;
    dirty_flag[i] = false;
  }
}

void loop() {
  // check for incoming MIDI messages and update voltage levels accordingly
  if (read_midi()) {
    if (midi_cc_ch1 != previous_voltage[0]) {
      dirty_flag[0] = true;
      previous_voltage[0] = midi_cc_ch1;
    }
    if (midi_cc_ch2 != previous_voltage[1]) {
      dirty_flag[1] = true;
      previous_voltage[1] = midi_cc_ch2;
    }
    // repeat for other channels...
  }

  // periodically check for dirty flags and update voltage levels
  for (int i = 0; i < NUM_CHANNELS; i++) {
    if (dirty_flag[i]) {
      update_voltage(i, previous_voltage[i]);
      dirty_flag[i] = false;
    }
  }

  // do other processing tasks...
}
