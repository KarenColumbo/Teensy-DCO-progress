void readADCAndBendNotes() {
  int adcValue = analogRead(ADC_PIN);
  float voltage = (adcValue / 4095.0) * ADC_MAX_VOLTAGE; // convert ADC value to voltage
  LFO_Detune = map(voltage, 0, ADC_MAX_VOLTAGE, -LFO_DETUNE_RANGE, LFO_DETUNE_RANGE);
  //bendNotes();
}