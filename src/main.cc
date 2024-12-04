#ifndef UNIT_TEST
#define IS_MAIN 1

// INCLUDES & MACROS

#include <Arduino.h>

#include "../test/config.h"

// Uncomment to enable debug
#include <debug.h>
#define DEBUG_SERIAL Serial // This file only

#include <chrono.h>
#include <I2CIP.h>
#include <wiipod.hpp>

#include <avr/wdt.h>

#define TIMESTAMP_LOG_DELTA_MS 5000 // Default
#define FSM_CYCLE_DELTA_MS     300  // 3 FPS
#define WIRENUM 0
#define MODULE  0
#define RENDER_SIZE_X  64
#define RENDER_SIZE_Y  36

// DECLARATIONS

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

int freeRam() {
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0  
    ? (int)&__heap_start : (int) __brkval);  
}

unsigned long lastCycle = 0;
void logCycle(bool _, const Number& cycle) {
  unsigned long delta = millis() - lastCycle;
  if(delta < FSM_CYCLE_DELTA_MS) {
    delay(FSM_CYCLE_DELTA_MS - delta);
  }
  lastCycle = millis();

  String m = _F("==== [ Cycle: ");
  m += cycle.toString();
  m += _F(" @ ");
  m += timestampToString(millis());
  if(delta < FSM_CYCLE_DELTA_MS) { m += "+ "; m += (FSM_CYCLE_DELTA_MS - delta); }
  m += " | SRAM: ";
  m += freeRam();
  m += _F(" ] ====");

  Serial.print(m);
}

// MAIN

void setup(void) {
  Serial.begin(115200);
  while(!Serial) { ; }
  cycle.addLoggerCallback(logCycle);
}

bool withinUnitCircle(double x, double y) {
  return (x * x) + (y * y) <= 1.0;
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
      WIIPOD_DEBUG_SERIAL.println(F("[WIIPOD] SETUP"));
      // WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
      // WIIPOD_DEBUG_SERIAL.println(F("s"));
    #endif
    wiipod = new WiiPod(WIRENUM, MODULE); return;
  }

  unsigned long now = millis();
  errlev = wiipod->update();
  switch(errlev) {
    case I2CIP_ERR_HARD:
      delete wiipod;
      wiipod = nullptr;
      break;
    case I2CIP_ERR_SOFT:
      break;
    default:
      errlev = wiipod->updateNunchuck(0, true);
      break;
  }
  unsigned long delta = millis() - now;
  DEBUG_SERIAL.print(F("[WIIPOD | I2CIP "));
  DEBUG_SERIAL.print(cycle.get().toString());
  DEBUG_SERIAL.print(F(" | "));
  DEBUG_SERIAL.print(1000.0 / delta, 0);
  DEBUG_SERIAL.print(F(" FPS | 0x"));
  DEBUG_SERIAL.print(errlev, HEX);
  DEBUG_SERIAL.println(F("]"));

  // RENDER UPDATE
  DEBUG_SERIAL.print(F("[WIIPOD | RENDER "));
  DEBUG_SERIAL.print(cycle.get().toString());
  DEBUG_SERIAL.print(F(" | "));
  DEBUG_SERIAL.print(fps, 0);
  DEBUG_SERIAL.print(F(" FPS | 0x"));
  DEBUG_SERIAL.print(errlev, HEX);
  DEBUG_SERIAL.println(F("]"));

  // 1. Clock, Cycle, Delay
  now = millis();
  if(errlev == I2CIP_ERR_NONE && wiipod != nullptr) {
    const wiipod_nunchuck_t* data = wiipod->getNunchuckCache();
    if(data != nullptr) {
      // Data & Fonts
      int _x = ((double)data->joy_x / 255.0) * RENDER_SIZE_X;
      int _y = ((255.0 - (double)data->joy_y) / 255.0) * RENDER_SIZE_Y; // Y-invert
      bool use_c = data->c;
      // bool use_z = data->z;
      bool border = data->z;
      // bool use_bold = use_c && use_z;
      // char c = use_bold ? ' ' : (use_z ? 'Z' : (use_c ? '+' : '~'));

      // Render Screen - 2:1 Aspect Ratio
      if(border){ DEBUG_SERIAL.print('|'); for(int x = 0; x < RENDER_SIZE_X; x++) { DEBUG_SERIAL.print('-'); } DEBUG_SERIAL.print('|'); }
      DEBUG_SERIAL.print('\n');

      for(int y = 0; y < RENDER_SIZE_Y; y++) {
        if(border) { DEBUG_SERIAL.print('|'); }
        for(int x = 0; x < RENDER_SIZE_X; x++) {
          if(x == _x && y == _y) {
            // DEBUG_SERIAL.print(use_bold ? 'X' : (use_z ? 'N' : (use_c ? '#' : '@')));
            DEBUG_SERIAL.print('X');
          } else if(use_c){
            double unit_x = 1.0 - ((2.0 * x) / RENDER_SIZE_X);
            double unit_y = 1.0 - ((2.0 * y) / RENDER_SIZE_Y);
            DEBUG_SERIAL.print(withinUnitCircle(unit_x, unit_y) ? ' ' :  '+');
          } else {
            DEBUG_SERIAL.print(' ');
          }
        }
        if(border) DEBUG_SERIAL.print('|');
        DEBUG_SERIAL.print('\n');
      }
      if(border){ DEBUG_SERIAL.print('|'); for(int x = 0; x < RENDER_SIZE_X; x++) { DEBUG_SERIAL.print('-'); } DEBUG_SERIAL.print('|'); }
    }

    // Get SHT31 Data
    // errlev = wiipod->updateSHT31(0, true);
    // if(errlev == I2CIP_ERR_NONE) {
    //   const state_sht31_t* sht31 = wiipod->getSHT31Cache();
    //   if(sht31 != nullptr) {
    //     DEBUG_SERIAL.print(F("[WIIPOD | SHT31] TEMPERATURE "));
    //     DEBUG_SERIAL.print(sht31->temperature, 2);
    //     DEBUG_SERIAL.print(F("C, HUMIDITY "));
    //     DEBUG_SERIAL.print(sht31->humidity, 2);
    //     DEBUG_SERIAL.println(F("% RH]"));
    //   }
    // }
  }

  fps = 1000.0 / (millis() - now);

  // delay(1000);
}

#endif