#ifndef UNIT_TEST
#define UNIT_TEST 1
#define IS_MAIN 1

// INCLUDES & MACROS

#include <Arduino.h>

#include "../test/config.h"

// Uncomment to enable debug
// #define DEBUG 0
#include <debug.h>
#define WIIPOD_DEBUG_SERIAL Serial // This file only

#include <chrono.h>
#include <I2CIP.h>
#include <wiipod.h>

// #include <avr/wdt.h>

#define TIMESTAMP_LOG_DELTA_MS 5000 // Default
#define FSM_CYCLE_DELTA_MS     3000  // 1 FPS
#define WIRENUM 0
#define MODULE  0

// DECLARATIONS
extern Chronograph Chronos;

using namespace I2CIP;

// Module* m;  // to be initialized in setup()
// Module* modules[I2CIP_MUX_COUNT] = { nullptr };
char idbuffer[10];

Variable cycle(Number(0, false, false), "Cycle");
void logCycle(bool _, const Number& cycle);

WiiPod* wiipod = nullptr;

// HELPERS

void crashout(void) {
  while(true) { // Blink
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

// int freeRam() {
//   extern int __heap_start,*__brkval;
//   int v;
//   return (int)&v - (__brkval == 0  
//     ? (int)&__heap_start : (int) __brkval);  
// }

unsigned long lastCycle = 0;
void logCycle(bool _, const Number& cycle) {
  unsigned long delta = millis() - lastCycle;
  #ifdef FSM_CYCLE_DELTA_MS
  if(delta < FSM_CYCLE_DELTA_MS) {
    delay(FSM_CYCLE_DELTA_MS - delta);
  }
  #endif
  lastCycle = millis();

  String m = _F("==== [ Cycle: ");
  m += cycle.toString();
  m += _F(" @ ");
  m += timestampToString(millis());
  // if(delta < FSM_CYCLE_DELTA_MS) { 
    m += " | ";
    // m += (FSM_CYCLE_DELTA_MS - delta);
    m += 1000.0 / delta;
    m += " FPS";
  // }
  // m += " | SRAM: ";
  // m += freeRam();
  m += _F(" ] ====");

  Serial.println(m);
}

// MAIN

void setup(void) {
  Serial.begin(115200);
  while(!Serial) { ; }
  cycle.addLoggerCallback(logCycle);
}

i2cip_errorlevel_t errlev;
double fps = 0.0;
bool flash = false;
void loop(void) {

  // FIXED UPDATE

  // 1. Clock, Cycle, Delay
  Chronos.set(millis()); cycle.set(cycle.get() + Number(1, false, false));

  // 2. I/O and OOP
  if(wiipod == nullptr) { 
    #ifdef WIIPOD_DEBUG_SERIAL
      WIIPOD_DEBUG_SERIAL.println(F("[WIIPOD | SETUP]"));
      // WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
      // WIIPOD_DEBUG_SERIAL.println(F("s"));
    #endif
    wiipod = new WiiPod(WIRENUM, MODULE); return;
  }

  unsigned long now = millis();
  errlev = wiipod->update();
  unsigned long delta = millis() - now;
  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | I2CIP "));
    WIIPOD_DEBUG_SERIAL.print(cycle.get().toString());
    WIIPOD_DEBUG_SERIAL.print(F(" | "));
    WIIPOD_DEBUG_SERIAL.print(1000.0 / delta, 0);
    WIIPOD_DEBUG_SERIAL.print(F(" FPS | 0x"));
    WIIPOD_DEBUG_SERIAL.print(errlev, HEX);
    WIIPOD_DEBUG_SERIAL.println(F("]"));
  #endif

  // delay(1000 / 30); // 30 FPS
}

#endif