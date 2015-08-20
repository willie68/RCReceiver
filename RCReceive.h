#include <inttypes.h>
/*
  RCReceive.h - Auslesen eine RC Empfängers - Version 0.2
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

// Dieser Schalter hat nur Auswirkungen bei einem ATTinyx5 Target.
// Damit wird anstatt der internen micros der Timer 1 verwendet.
//#define USE_TIMER_1

// Dieser Schalter hat nur Auswirkungen bei einem Arduino Mega (Mega128 oder Mega256) Target.
// Damit wird anstatt des Timer 1 der Timer 5 verwendet.
//#define USE_TIMER_5

// hier folgen nun die verschiedenen Definition für die verschiedenen Targets
// Der Arduinotyp kann dann mittels #if defined(xxx) abgefragt werden.
// Mögliche Werte:
// _RC_UNO_ : steht für alle 328 basierenden Arduinos, also Uno, Duemillanove, Mini, Micro, Nano
// _RC_LEONARDO : steht für alle 32U4 basierenden Arduinos, da wären Leonardo, oder auch die 32U4 Derivate von anderen  Herstellern
// _RC_MEGA_ : steht für die Arduinos Megas, also basierend auf dme ATMega128 bzw. ATMega256
// _RC_TINY_14_ : steht für die ATTinyx4 Serie 
// _RC_TINY_8_ : steht für die ATTinyx5 Serie 
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

// Konstanten für die RC Erkennung
const uint16_t MIN_RC_VALUE = 900; // minimaler Impuls, der noch als gültig erkannt wird
const uint16_t MAX_RC_VALUE = 2100; // maximaler Implus, der nnch als gültig erkannt wird
const uint8_t MAX_ERROR_COUNT = 3;

// Größe des Pufferspeichers
const uint8_t stackSize = 10; 

typedef struct  {
  volatile uint16_t stack[stackSize];
  // Laufvariable des Ringspeichers
  uint8_t index;
}
sStack;

// Die Klasse für den Empfänger
class RCReceive
{
public:
  static void initInt();

  RCReceive();

  // Empfngerklasse mit Pin verbinden
  void attach(uint8_t pin);

  // aktuellen gemittelten Wert besorgen
  uint8_t getValue();

  // neuen Wert vom Empfänger abfragen
  uint8_t poll();

  // Nullpunkt holen
  uint8_t getNP();

  // Nullpunkt holen (in us)
  uint16_t getMSNP();

  // Fehler Status, wird 1 wenn mehr als 3 fehlerhafte Impulse vom Empfänger ermittelt worden sind.
  uint8_t hasError();

  // Nullpunkt wurde bestimmt
  uint8_t hasNP();

  // Letzten gelesenen Wert ausgeben
  unsigned int getLastRCValue();

  // vereinfachung für die Interruptroutine
  void handleInterrupt();

  // Interruptbetrieb mit interner Iterruptroutie zuweisen
  void attachInt(uint8_t pin);

  // eigene Interruptroutine zuweisen
  void attachInt(void (*handler)(void));

  // Pin und eigene Interruptroutine zuweisen
  void attachInt(uint8_t pin, void (*handler)(void));

  // zugewiesenen Interruptroutine lösen
  void detachInt();

  // aktuellen gemittelten Wert in us besorgen
  uint16_t getMsValue();
  
  // us Wert in byte Wertebereich mappen
  uint8_t mapMsValue(uint16_t value);
  
protected:
  volatile unsigned int lastValue;
  sStack myStack;
  uint8_t myPin;
  uint8_t nullpoint;
  uint16_t msNullpoint;
  // die unteren 4 Bit geben die Anzahl der Fehler,
  // das oberste Bit gibt an, ob der Nullpunkt bestimmt wurde.
  volatile uint8_t state;
  volatile uint16_t RcTemp;
  bool hasValue;
  bool isIntMode;
  unsigned long lastValueTime;
};

#endif
