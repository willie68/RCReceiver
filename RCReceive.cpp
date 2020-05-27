#include "Arduino.h"
#include "wiring.c"
#include "pins_arduino.h"
#include "RCReceive.h"
#include "makros.h"
#include <avr/interrupt.h>
/*
  RCReceive.cpp - Reading an RC receiver - Version 0.2
 Copyright (c) 2014 Wilfried Klaas.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define ERROR_MASK 0x0F
#define HAS_NP 0x80

#define pushRcValue(value) \
if ((value > MIN_RC_VALUE) && (value < MAX_RC_VALUE)) { \
  this->myStack.stack[this->myStack.index % stackSize] = value; \
  this->myStack.index++; \
  this->state = (this->state & HAS_NP) ; \
} else { \
  this->state += 1; \
  if ((this->state & 0x10) > 0) { \
    this->state = 1;\
  }\
}

#define calcNP() \
if ((this->state & HAS_NP) == 0) { \
  if (myStack.index == stackSize) { \
    this->msNullpoint = getMsValue(); \
    this->nullpoint = mMapMsValue(this->msNullpoint); \
    this->state = this->state | HAS_NP; \
  } \
}

#define mMapMsValue(all) \
map(all, MIN_RC_VALUE, MAX_RC_VALUE, 0, 255)

static bool _RCReceiverIsInit = false;
static RCReceive* __ISR0Receiver, * __ISR1Receiver;

void __ISR_SWI_0 () {
  __ISR0Receiver->handleInterrupt();
}

void __ISR_SWI_1 () {
  __ISR1Receiver->handleInterrupt();
}

// Leonardo support
#if defined(_RC_MEGA_) || defined(_RC_LEONARDO_)
static RCReceive* __ISR2Receiver, * __ISR3Receiver;
void __ISR_SWI_2 () {
  __ISR2Receiver->handleInterrupt();
}
void __ISR_SWI_3 () {
  __ISR3Receiver->handleInterrupt();
}
#endif

// Mega support
#if defined(_RC_MEGA_)
static RCReceive* __ISR4Receiver, * __ISR5Receiver;
void __ISR_SWI_4 () {
  __ISR4Receiver->handleInterrupt();
}
void __ISR_SWI_5 () {
  __ISR5Receiver->handleInterrupt();
}
#endif

// general code without class reference, special for the ATTinyx5 series

#if defined(_RC_TINY_8_)
  #ifdef USE_TIMER_1
volatile uint8_t timer1_overflow_count;
uint16_t myMicros() {
  uint8_t m = timer1_overflow_count;
  uint8_t t = TCNT1;
  return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
}

ISR(TIMER1_OVF_vect)
{
  timer1_overflow_count++;
}
  #endif
#endif

// This is where the Receive class begins
RCReceive::RCReceive() {
  this->state = ERROR_MASK;
  this->nullpoint = 0;
}

// Prepare receiver for interrupt
void RCReceive::initInt() {
#if !defined(_RC_TINY_8_) && !defined(_RC_TINY_14_)
  if (!_RCReceiverIsInit) {
  #if defined(_RC_UNO_) || defined(_RC_LEONARDO_)
    TCNT1 = 0;              // clear the timer count
    // Initilialise Timer1
    TCCR1A = 0;             // normal counting mode
    TCCR1B = _BV(CS11);     // set prescaler of 8, tick = 0,5us same as in the Servo Lib...
    TIFR1 |= _BV(OCF1A);     // clear any pending interrupts;
    // OCR1A = TCNT1 + 4000; // Start in two milli seconds
  #endif
  #if defined(_RC_MEGA_)
    #if !defined(USE_TIMER_5)
    TCNT1 = 0;              // clear the timer count
    // Initilialise Timer1
    TCCR1A = 0;             // normal counting mode
    TCCR1B = _BV(CS11);     // set prescaler of 8, tick = 0,5us same as in the Servo Lib...
    TIFR1 |= _BV(OCF1A);     // clear any pending interrupts;
    // OCR1A = TCNT1 + 4000; // Start in two milli seconds
    #else
    TCNT5 = 0;              // clear the timer count
    // Initilialise Timer1
    TCCR5A = 0;             // normal counting mode
    TCCR5B = _BV(CS51);     // set prescaler of 8, tick = 0,5us same as in the Servo Lib...
    TIFR5 |= _BV(OCF5A);     // clear any pending interrupts;
    #endif
  #endif
    _RCReceiverIsInit = true;
  }
#endif
#if defined(_RC_TINY_8_)
  if (!_RCReceiverIsInit) {
    // timer 0 don't need to be initialised.
  #ifdef USE_TIMER_1
    TCNT1 = 0;              // clear the timer count
    // we are using the general timer 0 for this operation
    // Initilialise Timer1
    TCCR1 = 0x07;             // set prescaler of 64
    sbi(TIMSK, TOIE1); // Unlock interrupt
  #endif
    _RCReceiverIsInit = true;
  }
#endif
}

// Connect the receiver to the pin
void RCReceive::attach(uint8_t pin) {
  pinMode(pin, INPUT_PULLUP);
  this->myPin = pin;
  this->isIntMode = false;
}

// Get the currently averaged value
uint8_t RCReceive::getValue() {
  uint16_t all = getMsValue();
  return mMapMsValue(all);
}

uint8_t RCReceive::mapMsValue(uint16_t value) {
  return mMapMsValue(value);
}

// Get zero point
uint8_t RCReceive::getNP() {
  return this->nullpoint;
}

// get a new value from the recipient
// If 0 then recipient value is invalid
uint8_t RCReceive::poll() {
  lastValue = pulseIn(myPin, HIGH, 25000); // Timeout 100ms
  pushRcValue(lastValue);
  calcNP();
  return lastValue != 0;
}

// Error occurred
uint8_t RCReceive::hasError() {
  return ((state & ERROR_MASK) > 0);
}

// zero point determined
uint8_t RCReceive::hasNP() {
  return (state & HAS_NP) > 0;
}

unsigned int RCReceive::getLastRCValue() {
  uint16_t m = lastValue; 
  return m;
}

void RCReceive::handleInterrupt() {
  uint8_t sreg = SREG;
  cli();
  hasValue = true;
  if (digitalRead(myPin) ) { 
    // positive edge
#if defined(_RC_TINY_8_)
  #ifdef USE_TIMER_1
    RcTemp = myMicros(); 
  #else
    RcTemp = micros(); 
  #endif
#endif
#if defined(_RC_TINY_14_)
    RcTemp = micros(); 
#endif
#if defined(_RC_UNO_) || defined(_RC_LEONARDO_)
	RcTemp = TCNT1; 
#endif

#if defined(_RC_MEGA_) 
    #if defined(USE_TIMER_5)
      RcTemp = TCNT5;
    #else	
      RcTemp = TCNT1; 
    #endif
#endif
  } 
  else {
    // negative edge
#if defined(_RC_TINY_8_)
  #ifdef USE_TIMER_1
    lastValue = (myMicros() - RcTemp); 
  #else
    lastValue = (micros() - RcTemp); 
  #endif
#else
  #if defined(_RC_UNO_) || defined(_RC_LEONARDO_)
    lastValue = (TCNT1 - RcTemp) >> 1; 
  #endif
  #if defined(_RC_MEGA_)
    #if defined(USE_TIMER_5)
    lastValue = (TCNT5 - RcTemp) >> 1; 
    #else	
    lastValue = (TCNT1 - RcTemp) >> 1; 
    #endif
  #endif
#endif
    RcTemp = 0;
    pushRcValue(lastValue);
  }
  if ((state & HAS_NP) == 0) {
    calcNP();
  }
  SREG = sreg;
}

byte getIntFromPin(uint8_t pin) {
#if defined(_RC_LEONARDO_)
  switch (pin) {
  case 0:
    return 2;
    break;
  case 2:
    return 1;
    break;
  case 3:
    return 0;
    break;
  case 1:
    return 3;
    break;
  default:
    return 255;
  }
#elif defined(_RC_MEGA_)
  switch (pin) {
  case 2:
    return 0;
    break;
  case 3:
    return 1;
    break;
  case 21:
    return 2;
    break;
  case 20:
    return 3;
    break;
  case 19:
    return 4;
    break;
  case 18:
    return 5;
    break;
  default:
    return 255;
  }
#else
  return pin - 2;
#endif
}

void RCReceive::attachInt(void (*handler)(void)) {
  RCReceive::initInt();
  byte myInt = getIntFromPin(myPin);
  if (myInt < 10) {
    attachInterrupt(myInt, handler, CHANGE);
  }
}

void RCReceive::attachInt(uint8_t pin, void (*handler)(void)) {
  attach(pin);
  attachInt(handler);
}

void RCReceive::attachInt(uint8_t pin) {
  attach(pin);
  RCReceive::initInt();
  byte myInt = getIntFromPin(myPin);
  if (myInt < 10) {
    switch (myInt) {
    case 0:
      __ISR0Receiver = this;
      attachInterrupt(0, __ISR_SWI_0, CHANGE);
      break;
    case 1:
      __ISR1Receiver = this;
      attachInterrupt(1, __ISR_SWI_1, CHANGE);
      break;
#if defined(_RC_LEONARDO_) || defined(_RC_MEGA_)
    case 2:
      __ISR2Receiver = this;
      attachInterrupt(2, __ISR_SWI_2, CHANGE);
      break;
    case 3:
      __ISR3Receiver = this;
      attachInterrupt(3, __ISR_SWI_3, CHANGE);
      break;
#endif
#if defined(_RC_MEGA_)
    case 4:
      __ISR4Receiver = this;
      attachInterrupt(4, __ISR_SWI_4, CHANGE);
      break;
    case 5:
      __ISR5Receiver = this;
      attachInterrupt(5, __ISR_SWI_5, CHANGE);
      break;
#endif
    default:
      ;
    }
    this->isIntMode = true;
  }
}

void RCReceive::detachInt() {
  byte myInt = getIntFromPin(myPin);
  if (myInt < 10) {
    detachInterrupt(myInt);
  }
}

uint16_t RCReceive::getMsValue() {
  if (!hasValue) {
    if ((millis() - lastValueTime) > 100) {
      // last reception was more than 1 second ago, then error
      state += 1;
	  if ((state & 0x10) > 0) {
	    state = 1;
	  }
      lastValueTime = millis();
    }
  } 
  else {
    lastValueTime = millis();
  }
  hasValue = false;
  uint16_t all = 0;
  uint16_t minValue = myStack.stack[0];
  uint16_t maxValue = myStack.stack[0];
  all = myStack.stack[0];

  for (int i = 1; i < stackSize; i++) {
    uint16_t value = myStack.stack[i];
    // New maximum?
    if (value > maxValue) {
      // save new
      maxValue = value;
    } 
    // New lowest value?
    if (value < minValue) {
      // save new
      minValue = value;
    }
    all += value;
  }

  // Subtract the highest and lowest values
  all -= minValue;
  all -= maxValue;

  // form the mean
  all = all / (stackSize - 2);
  return all;
}
