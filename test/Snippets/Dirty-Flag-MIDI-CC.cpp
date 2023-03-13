const int NUM_CHANNELS = 32; // total number of MIDI CC channels
const int NUM_TCA = 2; // number of TCA9548 multiplexers
const int NUM_MCP_PER_TCA = 4; // number of MCP4728s per TCA9548
const int NUM_CH_PER_MCP = 4; // number of channels per MCP4728
const int NUM_MCP_CHANNELS = NUM_TCA * NUM_MCP_PER_TCA * NUM_CH_PER_MCP; // total number of MCP4728 channels

int previous_voltage[NUM_MCP_CHANNELS]; // array to store previous voltage levels
bool dirty_flag[NUM_MCP_CHANNELS]; // array to store dirty flag for each channel
int channel_map[NUM_CHANNELS]; // mapping array for translating MIDI CC channel numbers to MCP4728 channel numbers

// function to update the voltage level for a particular channel
void update_voltage(int channel, int voltage) {
  // send command to MCP4728 to update voltage level for the specified channel
  // ...
}

void setup() {
  // initialize previous voltage and dirty flag arrays
  for (int i = 0; i < NUM_MCP_CHANNELS; i++) {
    previous_voltage[i] = 0;
    dirty_flag[i] = false;
  }

  // initialize channel mapping array
  for (int i = 0; i < NUM_CHANNELS; i++) {
    int tca_index = i / (NUM_CH_PER_MCP * NUM_MCP_PER_TCA);
    int mcp_index = (i / NUM_CH_PER_MCP) % NUM_MCP_PER_TCA;
    int ch_index = i % NUM_CH_PER_MCP;
    int mcp_channel = mcp_index * NUM_CH_PER_MCP + ch_index;
    int tca_channel = tca_index * NUM_MCP_PER_TCA + mcp_index;
    channel_map[i] = tca_channel * NUM_CH_PER_MCP + mcp_channel;
  }
}

void loop() {
  // check for incoming MIDI messages and update voltage levels accordingly
  if (read_midi()) {
    for (int i = 0; i < NUM_CHANNELS; i++) {
      int mcp_channel = channel_map[i];
      if (midi_cc[i] != previous_voltage[mcp_channel]) {
        dirty_flag[mcp_channel] = true;
        previous_voltage[mcp_channel] = midi_cc[i];
      }
    }
  }

  // periodically check for dirty flags and update voltage levels
  for (int i = 0; i < NUM_MCP_CHANNELS; i++) {
    if (dirty_flag[i]) {
      update_voltage(i, previous_voltage[i]);
      dirty_flag[i] = false;
    }
  }

  // do other processing tasks...
}
