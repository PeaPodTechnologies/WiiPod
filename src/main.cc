#ifndef UNIT_TEST

#include <Arduino.h>

#include "../src/debug.h"
// #include <nunchuck.hpp>
// #include <sht31.h>
// #include <wiipod.hpp>

#include <chrono.h>
#include <I2CIP.h>

#include <avr/wdt.h>

#define TIMESTAMP_LOG_DELTA_MS 5000 // Default
#define FSM_CYCLE_DELTA_MS     1000
#define WIRENUM 0
#define MODULE  0

I2CIP::i2cip_errorlevel_t errlev;
Variable cycle(Number(0, false, false), "Cycle");
void logCycle(bool _, const Number& cycle);
// void logDisco(bool _, const bool& on);

// WiiPod* wiipod = nullptr;

char idbuffer[10];

void crashout(int err = -1) {
  Serial.print("CRASHOUT ");
  Serial.println(err, HEX);
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

void setup() {
  // Serial
  unsigned long now = millis();
  if(!Serial) { Serial.begin(115200); }
  unsigned long delta = millis() - now;
  #ifdef WIIPOD_DEBUG_SERIAL
    WIIPOD_DEBUG_SERIAL.print(F("[WIIPOD] SETUP "));
    WIIPOD_DEBUG_SERIAL.print(delta / 1000.0, 3);
    WIIPOD_DEBUG_SERIAL.println(F("s"));
  #endif

  // Construct States
  cycle.addLoggerCallback(logCycle);
  
  #ifdef _AVR_WDT_H_
    wdt_enable(WDTO_4S);
  #endif

  // wiipod = new WiiPod(WIRENUM, MODULE);
  
  // initializeModule();
}

void loop() {
  #ifdef _AVR_WDT_H_
    wdt_reset();
  #endif

  Chronos.set(millis());

  cycle.set(cycle.get() + Number(1, false, false));

  // Watchdog Timer Kickout
  // if(now > TWENTYFOURHRS_MILLIS) { while(true) { delay(1); } }


  // unsigned long i2cip_start = millis();
  // switch(wiipod->check()) {
  // // switch(wiipod.check()) {
  // // switch(checkModule(WIRENUM, MODULE)) {
  //   case I2CIP::I2CIP_ERR_HARD:
  //     // delete modules[MODULE];
  //     // modules[MODULE] = nullptr;
  //     crashout(I2CIP::I2CIP_ERR_HARD);
  //     return;
  //   case I2CIP::I2CIP_ERR_SOFT:
  //     if (!wiipod->initialize()) { 
  //     // if (!wiipod.initialize()) { 
  //     // if (!initializeModule(WIRENUM, MODULE)) { 
  //       // delete modules[MODULE]; modules[MODULE] = nullptr; return; }
  //       crashout(I2CIP::I2CIP_ERR_HARD); }
  //       // return; }
  //     break;
  //   default:
  //     // errlev = updateModule(WIRENUM, MODULE);
  //     errlev = wiipod->update();
  //     // errlev = wiipod.update();
  //     break;
  // }

  // if(errlev == I2CIP::I2CIP_ERR_NONE) {
  //   // updateNunchuck(true);
  // //   modules[MODULE]->operator()(nunchuck->getFQA(), true);
  //   // updateSHT31(true);
  // }

  // unsigned long i2cip_end = millis();

  // // DEBUG PRINT: CYCLE COUNT, FPS, and ERRLEV
  // Serial.print(F("[I2CIP | CYCLE "));
  // Serial.print(cycle.get().toString());
  // Serial.print(F(" | "));
  // Serial.print(1000.0 / (i2cip_end - i2cip_start), 0);
  // Serial.print(F(" FPS | 0x"));
  // Serial.print(errlev, HEX);
  // Serial.println(F("]"));
}

unsigned long lastCycle = 0;

void logCycle(bool _, const Number& cycle) {
  String m = _F("==== [ Cycle: ");
  m += cycle.toString();
  m += _F(" @ ");
  m += timestampToString(millis());
  m += "| SRAM: ";
  m += freeRam();
  m += _F(" ] ====");

  Serial.println(m);

  delay(FSM_CYCLE_DELTA_MS - (millis() - lastCycle));
  lastCycle = millis();

  // if(d > 0) {
  //   m += _F(" + ");
  //   m += d;
  //   m += _F("ms");
  // }
}

// i2cip_errorlevel_t updateModule(uint8_t wirenum, uint8_t modulenum) {



#endif