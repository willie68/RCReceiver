#include <inttypes.h>
/*
  RCReceive.h - Reading an RC receiver - Version 0.2
  Copyright (c) 2012 Wilfried Klaas.  All right reserved.

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
#ifndef RCReceive_h
#define RCReceive_h

// This switch only affects an ATTinyx5 target.
// This means that timer 1 is used instead of the internal micros.
//#define USE_TIMER_1

// This switch only affects an Arduino Mega (Mega128 or Mega256) target.
// This means that timer 5 is used instead of timer 1.
//#define USE_TIMER_5

// Here are the different definitions for the different targets
// The arduino type can then be queried using #if defined(xxx).
// Possible values:
// _RC_UNO_ : stands for all 328 based Arduinos, i.e.Uno, Duemillanove, Mini, Micro, Nano
// _RC_LEONARDO : stands for all 32U4 based Arduinos, there would be Leonardo, or also the 32U4 derivatives from other manufacturers
// _RC_MEGA_ : stands for the Arduinos Megas, i.e. based on dme ATMega128 or ATMega256
// _RC_TINY_14_ : stands for the ATTinyx4 series
// _RC_TINY_8_ : stands for the ATTinyx5 series
#if defined(__AVR_ATmega328P__)
#define _RC_UNO_
#endif

#if defined(__AVR_ATmega32U4__)
#define _RC_LEONARDO_
#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define _RC_MEGA_
#endif

#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
#define _RC_TINY_14_
#endif

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define _RC_TINY_8_
#endif

// Constants for RC detection
const uint16_t MIN_RC_VALUE = 900; // minimum pulse that is still recognized as valid
const uint16_t MAX_RC_VALUE = 2100; // maximum implus that is still recognized as valid
const uint8_t MAX_ERROR_COUNT = 3;

// size of the buffer memory
const uint8_t stackSize = 10;

typedef struct  {
  volatile uint16_t stack[stackSize];
  // Run variable of the ring buffer
  uint8_t index;
}
sStack;

// The class for the recipient
class RCReceive
{
public:
  static void initInt();

  RCReceive();

  // Connect receiver class with pin
  void attach(uint8_t pin);

  // get current averaged value
  uint8_t getValue();

  // query the new value from the recipient
  uint8_t poll();

  // Get zero point
  uint8_t getNP();

  // get zero point (in us)
  uint16_t getMSNP();

  // Error status, becomes 1 if more than 3 faulty pulses have been determined by the receiver.
  uint8_t hasError();

  // zero point was determined
  uint8_t hasNP();

  // Output the last value read
  unsigned int getLastRCValue();

  // simplification for the interrupt routine
  void handleInterrupt();

  // Assign interrupt mode with internal interrupt routine
  void attachInt(uint8_t pin);

  // assign your own interrupt routine
  void attachInt(void (*handler)(void));

  // Assign pin and own interrupt routine
  void attachInt(uint8_t pin, void (*handler)(void));

  // solve the assigned interrupt routine
  void detachInt();

  // get the current averaged value in us
  uint16_t getMsValue();

  // Map us value in byte value range
  uint8_t mapMsValue(uint16_t value);

protected:
  volatile unsigned int lastValue;
  sStack myStack;
  uint8_t myPin;
  uint8_t nullpoint;
  uint16_t msNullpoint;
  // the lower 4 bits indicate the number of errors,
  // The top bit indicates whether the zero point has been determined.
  volatile uint8_t state;
  volatile uint16_t RcTemp;
  bool hasValue;
  bool isIntMode;
  unsigned long lastValueTime;
};

#endif
