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

  // 1. Clock, Cycle, Delay
  if(errlev == I2CIP_ERR_NONE && wiipod != nullptr) {
  //   errlev = wiipod->updateNunchuck(WIIPOD_BUS_NUNCHUCK, true);
  //   const wiipod_nunchuck_t* data = wiipod->getNunchuckCache();
  //   if(errlev == I2CIP_ERR_NONE && data != nullptr) {
  //   //   // Data & Fonts
  //   //   // bool use_c = data->c;
  //   //   // bool use_z = data->z;
  //   //   // bool border = data->z;
  //   //   // bool use_bold = use_c && use_z;
  //   //   // char c = use_bold ? ' ' : (use_z ? 'Z' : (use_c ? '+' : '~'));

  //     // if(wiipod->screen == nullptr) {wiipod->screen = new Screen(I2CIP::createFQA(WIRENUM, MODULE, WIIPOD_BUS_SCREEN, I2CIP_SCREEN_ADDRESS));}

  //     // now = millis();
  //     // errlev = wiipod->renderNunchuck();
  //     // if(errlev == I2CIP_ERR_HARD) { delete wiipod->screen; wiipod->screen = nullptr; }
  //     // fps += 1000.0 / (millis() - now); fps /= 2.0; // Rolling average

  //     // // RENDER UPDATE
  //     // #ifdef WIIPOD_DEBUG_SERIAL
  //     //   wiipod->printNunchuck(WIIPOD_DEBUG_SERIAL); // Render Screen
  //     //   WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | RENDER "));
  //     //   WIIPOD_DEBUG_SERIAL.print(cycle.get().toString());
  //     //   WIIPOD_DEBUG_SERIAL.print(F(" | "));
  //     //   WIIPOD_DEBUG_SERIAL.print(fps, 0);
  //     //   WIIPOD_DEBUG_SERIAL.print(F(" FPS | 0x"));
  //     //   WIIPOD_DEBUG_SERIAL.print(errlev, HEX);
  //     //   WIIPOD_DEBUG_SERIAL.println(F("]"));
  //     // #endif
  //   }

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

    errlev = wiipod->updateSHT45(WIIPOD_BUS_SHT45, true);
    // if(errlev == I2CIP_ERR_NONE) {
    //   const state_sht45_t* sht45 = wiipod->getSHT45Cache();
    //   if(sht45 != nullptr) {
    //     #ifdef WIIPOD_DEBUG_SERIAL
    //       WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | SHT45] TEMPERATURE "));
    //       WIIPOD_DEBUG_SERIAL.print(sht45->temperature, 1);
    //       WIIPOD_DEBUG_SERIAL.print(F("C, HUMIDITY "));
    //       WIIPOD_DEBUG_SERIAL.print(sht45->humidity, 1);
    //       WIIPOD_DEBUG_SERIAL.println(F("%"));
    //     #endif
    //   }
    // }

    errlev = wiipod->updateK30(WIIPOD_BUS_K30, true);
    // if(errlev == I2CIP_ERR_NONE) {
    //   const uint16_t* k30 = wiipod->getK30Cache();
    //   if(k30 != nullptr) {
    //     WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | K30] CO2 "));
    //     WIIPOD_DEBUG_SERIAL.print(*k30);
    //     WIIPOD_DEBUG_SERIAL.print(F(" PPM"));
    //   }
    // }

    errlev = wiipod->updateMCP23017(WIIPOD_BUS_MCP, true);

    errlev = wiipod->updateRotaryEncoder(WIIPOD_BUS_ROTARY, true);
    if(errlev == I2CIP_ERR_NONE) {
      const i2cip_rotaryencoder_t* seesaw = wiipod->getSeesawCache();
      if(seesaw != nullptr) {
        if(wiipod->screen == nullptr) {wiipod->screen = new SSD1306(I2CIP::createFQA(WIRENUM, MODULE, WIIPOD_BUS_SCREEN, I2CIP_SSD1306_ADDRESS));}
        if(wiipod->screen != nullptr) {
          now = millis();
          errlev = wiipod->renderRotary();
          if(errlev != I2CIP_ERR_NONE) { 
            #ifdef WIIPOD_DEBUG_SERIAL
              WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD | RENDER] ROTARY -> SCREEN FAIL "));
              WIIPOD_DEBUG_SERIAL.print(errlev, HEX);
            #endif
          }
        }
      }
    }

  // Uncomment to perform scan
    // #ifdef WIIPOD_DEBUG_SERIAL
    //   wiipod->scanToPrint(WIIPOD_DEBUG_SERIAL, WIRENUM, MODULE);
    // #endif
  }

  // delay(1000 / 30); // 30 FPS
}

#endif