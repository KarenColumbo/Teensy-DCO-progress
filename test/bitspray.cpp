// -------------------- Write bent note frequency voltages to Note GPIOs
  analogWrite(2, voices[0].bentNote);
  analogWrite(3, voices[1].bentNote);
  analogWrite(4, voices[2].bentNote);
  analogWrite(5, voices[3].bentNote);
  analogWrite(6, voices[4].bentNote);
  analogWrite(9, voices[5].bentNote);
  analogWrite(22, voices[6].bentNote);
  analogWrite(23, voices[7].bentNote);


// ----- Bit Spraying:
for (int i=0; i<8; i++) {
   uint16_t writeFrequency = voices[i].bentNote;
 
   digitalWriteFast(2 + i, writeFrequency & 0b01);
   digitalWriteFast(2 + i, (writeFrequency & 0b10) >> 1);
   digitalWriteFast(2 + i, (writeFrequency & 0b100) >> 2);
   digitalWriteFast(2 + i, (writeFrequency & 0b1000) >> 3);
   digitalWriteFast(2 + i, (writeFrequency & 0b10000) >> 4);
   digitalWriteFast(2 + i, (writeFrequency & 0b100000) >> 5);
   digitalWriteFast(2 + i, (writeFrequency & 0b1000000) >> 6);
   digitalWriteFast(2 + i, (writeFrequency & 0b10000000) >> 7);
   digitalWriteFast(2 + i, (writeFrequency & 0b100000000) >> 8);
   digitalWriteFast(2 + i, (writeFrequency & 0b1000000000) >> 9);
   digitalWriteFast(2 + i, (writeFrequency & 0b10000000000) >> 10);
   digitalWriteFast(2 + i, (writeFrequency & 0b100000000000) >> 11);
   digitalWriteFast(2 + i, (writeFrequency & 0b1000000000000) >> 12);
   digitalWriteFast(2 + i, (writeFrequency & 0b10000000000000) >> 13);
}

#include <HardwareSerial.h>
#include <wiring_private.h>

// declare a pointer to the register that contains all the
// bit masks for GPIO pins
volatile uint32_t* pPORTB = portOutputRegister(2);

// function for writing a 14-bit value to an 8-bit pin
void digitalWrite14L(uint8_t pin, uint32_t value) {
  uint32_t mask;
  // first 8 bits are normal pin values
  digitalWrite8(pin, (value & 0xff));
  // next 6 bits are in port B, left aligned
  mask = (63 << (pin&0b111));  // build a mask of the appropriate number and location of bits
  *pPORTB = (*pPORTB & ~mask) | ((value >> 8) << (pin&0b111));
}

// write the 8 values
digitalWrite14L(2, voices[0].bentNote);
digitalWrite14L(3, voices[1].bentNote);
digitalWrite14L(4, voices[2].bentNote);
digitalWrite14L(5, voices[3].bentNote);
digitalWrite14L(6, voices[4].bentNote);
digitalWrite14L(9, voices[5].bentNote);
digitalWrite14L(22, voices[6].bentNote);
digitalWrite14L(23, voices[7].bentNote);




#include "GPIOConstants.h"

void bb_writepulse(volatile uint32_t * regAddr, int bitIndex, uint16_t dutyCycle) {
  // Clear the pin
  *regAddr = *regAddr & ~(1 << bitIndex);

  // Delay for off period
  delay(dutyCycle);
  
  // Set the pin
  *regAddr = *regAddr | (1 << bitIndex);
  
  // Delay for on period
  delay(14 - dutyCycle);
}

void writeVoltages(uint16_t * bentNote) {
  bb_writepulse(GPIO_PCOR_REG(GPIOF_BASE_PTR), 2, bentNote[0]);
  bb_writepulse(GPIO_PCOR_REG(GPIOD_BASE_PTR), 3, bentNote[1]);
  bb_writepulse(GPIO_PCOR_REG(GPIOD_BASE_PTR), 4, bentNote[2]);
  bb_writepulse(GPIO_PCOR_REG(GPIOD_BASE_PTR), 5, bentNote[3]);
  bb_writepulse(GPIO_PCOR_REG(GPIOE_BASE_PTR), 0, bentNote[4]);
  bb_writepulse(GPIO_PCOR_REG(GPIOD_BASE_PTR), 8, bentNote[5]);
  bb_writepulse(GPIO_PCOR_REG(GPIOC_BASE_PTR), 6, bentNote[6]);
  bb_writepulse(GPIO_PCOR_REG(GPIOB_BASE_PTR), 21, bentNote[7]);
}


// Teensy 4.1 C++ code 
#include <Arduino.h>
#include <stdint.h>

uint16_t voices[8].bentNote;

//define pins 
#define pinA 2
#define pinB 3 
#define pinC 4 
#define pinD 5 
#define pinE 6 
#define pinF 9 
#define pinG 22 
#define pinH 23

void writePwmOut(uint16_t value) {

// write low byte
  for (uint8_t i = 0; i < 8; i++) {
      digitalWrite(pinA + i, !!(value & (1 << i))); 
  }

  //write high byte
  for (uint8_t i = 0; i < 6; i++) {
      digitalWrite(pinA + 8 + i, !!(value & (1 << (8 + i))));
  }
}

//loop to write 8 voltages 
  for (uint8_t i = 0; i < 8; i++) {
    writePwmOut(voices[i].bentNote); 
  }