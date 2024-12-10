#ifndef UNIT_TEST
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
// #define FSM_CYCLE_DELTA_MS     300  // 3 FPS
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
  // if(delta < FSM_CYCLE_DELTA_MS) {
  //   delay(FSM_CYCLE_DELTA_MS - delta);
  // }
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
  switch(errlev) {
    case I2CIP_ERR_HARD:
      // delete wiipod;
      // wiipod = nullptr;
      break;
    case I2CIP_ERR_SOFT:
      break;
    default:
      errlev = wiipod->updateNunchuck(0, true);
      break;
  }
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

  // 1. Clock, Cycle, Delay
  now = millis();
  if(errlev == I2CIP_ERR_NONE && wiipod != nullptr) {
    const wiipod_nunchuck_t* data = wiipod->getNunchuckCache();
    if(data != nullptr) {
      // Data & Fonts
      // bool use_c = data->c;
      // bool use_z = data->z;
      // bool border = data->z;
      // bool use_bold = use_c && use_z;
      // char c = use_bold ? ' ' : (use_z ? 'Z' : (use_c ? '+' : '~'));

      // wiipod->printNunchuck(WIIPOD_DEBUG_SERIAL); // Render Screen

      // RENDER UPDATE
      #ifdef WIIPOD_DEBUG_SERIAL
        wiipod->printNunchuck(WIIPOD_DEBUG_SERIAL); // Render Screen
        WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | RENDER "));
        WIIPOD_DEBUG_SERIAL.print(cycle.get().toString());
        WIIPOD_DEBUG_SERIAL.print(F(" | "));
        WIIPOD_DEBUG_SERIAL.print(fps, 0);
        WIIPOD_DEBUG_SERIAL.print(F(" FPS | 0x"));
        WIIPOD_DEBUG_SERIAL.print(errlev, HEX);
        WIIPOD_DEBUG_SERIAL.println(F("]"));
      #endif
    }

    // Get SHT31 Data
    // errlev = wiipod->updateSHT31(1, true);
    // if(errlev == I2CIP_ERR_NONE) {
    //   const state_sht31_t* sht31 = wiipod->getSHT31Cache();
    //   if(sht31 != nullptr) {
    //     WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | SHT31] TEMPERATURE "));
    //     WIIPOD_DEBUG_SERIAL.print(sht31->temperature, 2);
    //     WIIPOD_DEBUG_SERIAL.print(F("C, HUMIDITY "));
    //     WIIPOD_DEBUG_SERIAL.print(sht31->humidity, 2);
    //     WIIPOD_DEBUG_SERIAL.println(F("% RH"));
    //   }
    // }

    errlev = wiipod->updateSHT45(0, true);
    if(errlev == I2CIP_ERR_NONE) {
      const state_sht45_t* sht45 = wiipod->getSHT45Cache();
      if(sht45 != nullptr) {
        #ifdef WIIPOD_DEBUG_SERIAL
          WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | SHT45] TEMPERATURE "));
          WIIPOD_DEBUG_SERIAL.print(sht45->temperature, 1);
          WIIPOD_DEBUG_SERIAL.print(F("C, HUMIDITY "));
          WIIPOD_DEBUG_SERIAL.print(sht45->humidity, 1);
          WIIPOD_DEBUG_SERIAL.println(F("%"));
        #endif
      }
    }

    errlev = wiipod->updateK30(1, true);
    if(errlev == I2CIP_ERR_NONE) {
      const uint16_t* k30 = wiipod->getK30Cache();
      if(k30 != nullptr) {
        WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | K30] CO2 "));
        WIIPOD_DEBUG_SERIAL.print(*k30);
        WIIPOD_DEBUG_SERIAL.print(F(" PPM"));
      }
    }
  }

  fps += 1000.0 / (millis() - now); fps /= 2.0; // Rolling average

  // delay(1000 / 30); // 30 FPS
}

#endif